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

#include <AK/Array.h>
#include <AK/MemoryStream.h>
#include <AK/Optional.h>
#include <AK/StdLibExtras.h>

// #define DEBUG_CANONICAL_CODE

namespace Compress::New {

class Reader {
public:
    // Padded with zeroes if not enough bits are avaliable.
    u16 peek() const
    {
        try_to_buffer_16bits();
        return m_buffered & 0xffff;
    }

    void consume(size_t bits)
    {
        try_to_buffer_16bits();
#ifdef DEBUG_CANONICAL_CODE
        ASSERT(bits <= m_buffered_bits);
#endif
        m_buffered_bits -= min(bits, m_buffered_bits);
    }

private:
    size_t try_read_two_bytes(u16& value);

    void try_to_buffer_16bits() const
    {
        // FIXME: Verify byte / bit order.

        if (m_buffered_bits >= 16)
            return;

        if (m_stream.remaining() >= 2) {
            u16 value;
            m_stream >> value;

            m_buffered |= static_cast<u32>(value) << m_buffered_bits;
            m_buffered_bits += 2;
        } else if (m_stream.remaining() == 1) {
            u8 value;
            m_stream >> value;

            m_buffered |= static_cast<u32>(value) << m_buffered_bits;
            m_buffered_bits += 1;
        }
    }

    // FIXME: This obviously can't stay this way. The logic for this should be added to this stream.
    mutable InputMemoryStream& m_stream;

    mutable u32 m_buffered;
    mutable size_t m_buffered_bits { 0 };
};

// This is very much inspired by "Efficient Huffman Decoding" in https://www.hanshq.net/zip.html.
class CanonicalCode {
public:
    u16 read_symbol()
    {
        const auto bits = m_reader.peek();

        for (size_t idx = 0; idx < min<size_t>(m_bits_used, 15); ++idx) {
            if (bits < m_sentinel_bits[idx]) {
                m_reader.consume(idx + 1);
                return m_first_codeword_offsets[idx] + bits;
            }
        }

        ASSERT_NOT_REACHED();
    }

private:
    Reader m_reader;
    Array<u16, 15> m_sentinel_bits;
    Array<i16, 15> m_first_codeword_offsets;
    u8 m_bits_used;
};

}
