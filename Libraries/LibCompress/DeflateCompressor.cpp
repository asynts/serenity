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

#include <AK/BinarySearch.h>
#include <AK/Function.h>
#include <AK/HashMap.h>
#include <AK/OwnPtr.h>
#include <AK/QuickSort.h>
#include <AK/Vector.h>
#include <sys/mman.h>

#include <LibCompress/DeflateCompressor.h>

namespace Compress {

void DeflateCompressor::allocate_buffer()
{
    void* ptr = ::mmap(nullptr, 128 * KiB, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
    ASSERT(ptr != MAP_FAILED);

    m_buffer = { ptr, 128 * KiB };
}

void DeflateCompressor::deallocate_buffer()
{
    int retval = ::munmap(m_buffer.data(), m_buffer.size());
    ASSERT(retval == 0);
}

struct CodeNode {
    u16 symbol;
    size_t frequency;

    OwnPtr<CodeNode> left = nullptr;
    OwnPtr<CodeNode> right = nullptr;
};

void DeflateCompressor::flush_buffer()
{
    constexpr u16 invalid_symbol = NumericLimits<u16>::max();

    // Count symbol frequency in buffer.
    Vector<NonnullOwnPtr<CodeNode>, 256> nodes;
    for (u16 i = 0; i < 256; ++i)
        nodes.append(make<CodeNode>(i, 0));
    nodes.append(make<CodeNode>(0, 1));
    for (size_t i = 0; i < m_buffer_used; ++i)
        ++nodes[m_buffer[i]]->frequency;

    // We sort the list and keep it sorted further down the line.
    AK::quick_sort(nodes, [](auto& lhs, auto& rhs) { return lhs->frequency < rhs->frequency; });

    // Create a Huffman code.
    while (nodes.size() >= 2) {
        auto left = nodes.take(0);
        auto right = nodes.take(1);

        auto joined = make<CodeNode>(invalid_symbol, left->frequency + right->frequency, move(left), move(right));

        size_t nearby_index = 0;
        AK::binary_search(
            nodes,
            joined,
            &nearby_index,
            [](auto& lhs, auto& rhs) { return static_cast<int>(lhs->frequency - rhs->frequency); });

        nodes.insert(nearby_index, move(joined));
    }

    // Group the symbols by code length.
    HashMap<u8, Vector<u16>> codewords_by_length;

    Function<void(CodeNode*, u8)> visitor;
    visitor = [&](CodeNode* node, u8 length) {
        if (node->symbol != invalid_symbol)
            codewords_by_length.ensure(length).append(node->symbol);

        if (node->left)
            visitor(node->left, length + 1);

        if (node->right)
            visitor(node->right, length + 1);
    };

    ASSERT(nodes.size() == 1);
    visitor(nodes[0], 1);

    // Extract the code in ascending order.
    HashMap<u16, u16> code;

    // We put a leading one to be able to tell the bit length.
    u16 last_symbol = 1;
    for (auto& codewords : codewords_by_length) {
        AK::quick_sort(codewords.value);

        last_symbol <<= 1;
        for (auto symbol : codewords.value)
            code.set(symbol, last_symbol++);
    }

    // FIXME: Write header with code...

    // Encode the block onto the stream.
    for (u8 byte : m_buffer.trim(m_buffer_used))
        m_stream.write_bits_implicit(code.get(byte).value());
    m_stream.align_to_byte_boundary_with_zero_fill();
}

}
