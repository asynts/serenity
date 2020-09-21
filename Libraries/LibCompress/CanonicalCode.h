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

namespace Compress::New {

// FIXME: Verify byte/bit order.

// FIXME: We want to read from an arbitrary stream.
class InputBitStream {
public:
    explicit InputBitStream(ReadonlyBytes bytes)
        : m_stream(bytes)
    {
    }

    u16 peek() const
    {
        if (m_buffered_bits < 16) {
            if (m_stream.remaining() >= 2) {
                LittleEndian<u16> value;
                m_stream >> value;

                m_buffered |= value << m_buffered_bits;
                m_buffered_bits += 16;
            } else if (m_stream.remaining() == 1) {
                LittleEndian<u8> value;
                m_stream >> value;

                m_buffered |= value << m_buffered_bits;
                m_buffered_bits += 8;
            }
        }

        return m_buffered & 0xffff;
    }

    void consume(size_t nbits)
    {
        m_buffered >>= nbits;
        m_buffered_bits -= nbits;
    }

    bool eof() const { return m_buffered_bits == 0 && m_stream.eof(); }

private:
    mutable u32 m_buffered;
    mutable size_t m_buffered_bits { 0 };

    mutable InputMemoryStream m_stream;
};

// This is very much inspired by "Efficient Huffman Decoding" in https://www.hanshq.net/zip.html.
class CanonicalCode {
public:
    // FIXME: We do absolutely no error checking here.
    static Optional<CanonicalCode> from_bytes(Span<const u8> symbol_lengths)
    {
        CanonicalCode code;

        Array<u8, 15> lengths { 0 };
        for (auto length : symbol_lengths)
            ++lengths[length - 1];

        u16 first_unused_code_word = 0;
        for (size_t length_index = 0; length_index < 15; ++length_index) {
            if (lengths[length_index] == 0)
                continue;

            code.m_max_length_index = length_index;

            u16 first_symbol = 0;
            for (; first_symbol < symbol_lengths.size(); ++first_symbol) {
                if (symbol_lengths[first_symbol] == length_index + 1)
                    break;
            }

            code.m_first_codeword_offsets[length_index] = static_cast<i16>(first_symbol) - static_cast<i16>(first_unused_code_word);
            code.m_sentinel_bits[length_index] = first_unused_code_word + lengths[length_index];

            first_unused_code_word += lengths[length_index];
        }

        return code;
    }

    u16 read_symbol(InputBitStream& stream)
    {
        const auto bits = stream.peek();

        for (size_t length_index = 0; length_index <= min<size_t>(m_max_length_index, 14); ++length_index) {
            if (bits < m_sentinel_bits[length_index]) {
                stream.consume(length_index + 1);
                return m_first_codeword_offsets[length_index] + bits;
            }
        }

        ASSERT_NOT_REACHED();
    }

    bool unreliabe_eof() const { TODO(); }

private:
    Array<u16, 15> m_sentinel_bits { 0 };
    Array<i16, 15> m_first_codeword_offsets { 0 };
    u8 m_max_length_index { 0 };
};

}
