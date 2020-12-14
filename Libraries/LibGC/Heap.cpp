/*
 * Copyright (c) 2020, the SerenityOS developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <AK/Badge.h>
#include <AK/HashTable.h>
#include <AK/StackInfo.h>
#include <AK/TemporaryChange.h>
#include <LibCore/ElapsedTimer.h>
#include <LibGC/Allocator.h>
#include <LibGC/Heap.h>
#include <LibGC/HeapBlock.h>
#include <setjmp.h>

//#define HEAP_DEBUG

namespace GC {

Heap::Heap()
{
    m_allocators.append(make<Allocator>(16));
    m_allocators.append(make<Allocator>(32));
    m_allocators.append(make<Allocator>(64));
    m_allocators.append(make<Allocator>(128));
    m_allocators.append(make<Allocator>(256));
    m_allocators.append(make<Allocator>(512));
    m_allocators.append(make<Allocator>(1024));
    m_allocators.append(make<Allocator>(3172));
}

Heap::~Heap()
{
    collect_garbage(CollectionType::CollectEverything);
}

ALWAYS_INLINE Allocator& Heap::allocator_for_size(size_t cell_size)
{
    for (auto& allocator : m_allocators) {
        if (allocator->cell_size() >= cell_size)
            return *allocator;
    }
    ASSERT_NOT_REACHED();
}

Cell* Heap::allocate_cell(size_t size)
{
    if (should_collect_on_every_allocation()) {
        collect_garbage();
    } else if (m_allocations_since_last_gc > m_max_allocations_between_gc) {
        m_allocations_since_last_gc = 0;
        collect_garbage();
    } else {
        ++m_allocations_since_last_gc;
    }

    auto& allocator = allocator_for_size(size);
    return allocator.allocate_cell(*this);
}

void Heap::collect_garbage(CollectionType collection_type, bool print_report)
{
    ASSERT(!m_collecting_garbage);
    TemporaryChange change(m_collecting_garbage, true);

    Core::ElapsedTimer collection_measurement_timer;
    collection_measurement_timer.start();
    if (collection_type == CollectionType::CollectGarbage) {
        if (m_gc_deferrals) {
            m_should_gc_when_deferral_ends = true;
            return;
        }
        HashTable<Cell*> roots;
        gather_roots(roots);
        mark_live_cells(roots);
    }
    sweep_dead_cells(print_report, collection_measurement_timer);
}

void Heap::gather_roots(HashTable<Cell*>& roots)
{
    TODO();

#ifdef HEAP_DEBUG
    dbgln("gather_roots:");
    for (auto* root : roots)
        dbgln("  + {}", root);
#endif
}

void Heap::gather_conservative_roots(HashTable<Cell*>& roots)
{
    TODO();
}

class MarkingVisitor final : public Cell::Visitor {
public:
    MarkingVisitor() { }

    virtual void visit_impl(Cell* cell)
    {
        if (cell->is_marked())
            return;
#ifdef HEAP_DEBUG
        dbgln("  ! {}", cell);
#endif
        cell->set_marked(true);
        cell->visit_edges(*this);
    }
};

void Heap::mark_live_cells(const HashTable<Cell*>& roots)
{
#ifdef HEAP_DEBUG
    dbgln("mark_live_cells:");
#endif
    MarkingVisitor visitor;
    for (auto* root : roots)
        visitor.visit(root);
}

void Heap::sweep_dead_cells(bool print_report, const Core::ElapsedTimer& measurement_timer)
{
#ifdef HEAP_DEBUG
    dbgln("sweep_dead_cells:");
#endif
    Vector<HeapBlock*, 32> empty_blocks;
    Vector<HeapBlock*, 32> full_blocks_that_became_usable;

    size_t collected_cells = 0;
    size_t live_cells = 0;
    size_t collected_cell_bytes = 0;
    size_t live_cell_bytes = 0;

    for_each_block([&](auto& block) {
        bool block_has_live_cells = false;
        bool block_was_full = block.is_full();
        block.for_each_cell([&](Cell* cell) {
            if (cell->is_live()) {
                if (!cell->is_marked()) {
#ifdef HEAP_DEBUG
                    dbgln("  ~ {}", cell);
#endif
                    block.deallocate(cell);
                    ++collected_cells;
                    collected_cell_bytes += block.cell_size();
                } else {
                    cell->set_marked(false);
                    block_has_live_cells = true;
                    ++live_cells;
                    live_cell_bytes += block.cell_size();
                }
            }
        });
        if (!block_has_live_cells)
            empty_blocks.append(&block);
        else if (block_was_full != block.is_full())
            full_blocks_that_became_usable.append(&block);
        return IterationDecision::Continue;
    });

    for (auto* block : empty_blocks) {
#ifdef HEAP_DEBUG
        dbgln(" - HeapBlock empty @ {}: cell_size={}", block, block->cell_size());
#endif
        allocator_for_size(block->cell_size()).block_did_become_empty({}, *block);
    }

    for (auto* block : full_blocks_that_became_usable) {
#ifdef HEAP_DEBUG
        dbgln(" - HeapBlock usable again @ {}: cell_size={}", block, block->cell_size());
#endif
        allocator_for_size(block->cell_size()).block_did_become_usable({}, *block);
    }

#ifdef HEAP_DEBUG
    for_each_block([&](auto& block) {
        dbgln(" > Live HeapBlock @ {}: cell_size={}", &block, block.cell_size());
        return IterationDecision::Continue;
    });
#endif

    int time_spent = measurement_timer.elapsed();

    if (print_report) {
        size_t live_block_count = 0;
        for_each_block([&](auto&) {
            ++live_block_count;
            return IterationDecision::Continue;
        });

        dbgln("Garbage collection report");
        dbgln("=============================================");
        dbgln("     Time spent: {} ms", time_spent);
        dbgln("     Live cells: {} ({} bytes)", live_cells, live_cell_bytes);
        dbgln("Collected cells: {} ({} bytes)", collected_cells, collected_cell_bytes);
        dbgln("    Live blocks: {} ({} bytes)", live_block_count, live_block_count * HeapBlock::block_size);
        dbgln("   Freed blocks: {} ({} bytes)", empty_blocks.size(), empty_blocks.size() * HeapBlock::block_size);
        dbgln("=============================================");
    }
}

void Heap::defer_gc(Badge<Defer>)
{
    ++m_gc_deferrals;
}

void Heap::undefer_gc(Badge<Defer>)
{
    ASSERT(m_gc_deferrals > 0);
    --m_gc_deferrals;

    if (!m_gc_deferrals) {
        if (m_should_gc_when_deferral_ends)
            collect_garbage();
        m_should_gc_when_deferral_ends = false;
    }
}

}
