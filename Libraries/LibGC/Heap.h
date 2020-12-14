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

#pragma once

#include <AK/Badge.h>
#include <AK/HashTable.h>
#include <AK/Noncopyable.h>
#include <AK/NonnullOwnPtr.h>
#include <AK/Types.h>
#include <AK/Vector.h>
#include <LibCore/ElapsedTimer.h>
#include <LibGC/Forward.h>

namespace GC {

class Heap {
    AK_MAKE_NONCOPYABLE(Heap);
    AK_MAKE_NONMOVABLE(Heap);

public:
    explicit Heap();
    ~Heap();

    template<typename T, typename... Args>
    T* allocate(Args&&... args)
    {
        auto* memory = allocate_cell(sizeof(T));
        new (memory) T(forward<Args>(args)...);
        return static_cast<T*>(memory);
    }

    enum class CollectionType {
        CollectGarbage,
        CollectEverything,
    };

    void collect_garbage(CollectionType = CollectionType::CollectGarbage, bool print_report = false);

    bool should_collect_on_every_allocation() const { return m_should_collect_on_every_allocation; }
    void set_should_collect_on_every_allocation(bool b) { m_should_collect_on_every_allocation = b; }

    void defer_gc(Badge<Defer>);
    void undefer_gc(Badge<Defer>);

private:
    Cell* allocate_cell(size_t);

    void gather_roots(HashTable<Cell*>&);
    void gather_conservative_roots(HashTable<Cell*>&);
    void mark_live_cells(const HashTable<Cell*>& live_cells);
    void sweep_dead_cells(bool print_report, const Core::ElapsedTimer&);

    Allocator& allocator_for_size(size_t);

    template<typename Callback>
    void for_each_block(Callback callback)
    {
        for (auto& allocator : m_allocators) {
            if (allocator->for_each_block(callback) == IterationDecision::Break)
                return;
        }
    }

    size_t m_max_allocations_between_gc = 10000;
    size_t m_allocations_since_last_gc = 0;

    bool m_should_collect_on_every_allocation = false;

    Vector<NonnullOwnPtr<Allocator>> m_allocators;

    size_t m_gc_deferrals = 0;
    bool m_should_gc_when_deferral_ends { false };

    bool m_collecting_garbage { false };
};

}
