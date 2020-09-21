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

// FIXME: Put a large circular buffer into this class.
class Reader {
public:
    // Padded with zeroes if not enough bits are avaliable.
    u16 peek() const;

    void consume(size_t bits);
};

// This is very much inspired by "Efficient Huffman Decoding" in https://www.hanshq.net/zip.html.
class CanonicalCode {
public:
    // FIXME: Analyse the behavior of this constructor for invalid symbol lengths. Ensure that
    //        whatever the output is, doesn't cause any crashes.
    explicit CanonicalCode(Span<const u8> symbol_lengths)
    {
        Array<u8, 15> lengths { 0 };

        for (auto length : symbol_lengths) {
            ++lengths[length - 1];
        }

        u16 first_unused_code_word = 0;
        for (size_t length_index = 0; length_index < 15; ++length_index) {
            if (lengths[length_index] == 0)
                continue;

            m_max_length_index = length_index;

            u16 first_symbol = 0;
            for (; first_symbol < symbol_lengths.size(); ++first_symbol) {
                if (symbol_lengths[first_symbol] == length_index)
                    break;
            }

            m_first_codeword_offsets[length_index] = static_cast<i16>(first_symbol) - static_cast<i16>(first_unused_code_word);
            m_sentinel_bits[length_index] = first_unused_code_word + lengths[length_index];

            first_unused_code_word += lengths[length_index];
        }
    }

    u16 read_symbol()
    {
        const auto bits = m_reader.peek();

        for (size_t length_index = 0; length_index <= min<size_t>(m_max_length_index, 14); ++length_index) {
            if (bits < m_sentinel_bits[length_index]) {
                m_reader.consume(length_index + 1);
                return m_first_codeword_offsets[length_index] + bits;
            }
        }

        ASSERT_NOT_REACHED();
    }

private:
    Reader m_reader;
    Array<u16, 15> m_sentinel_bits;
    Array<i16, 15> m_first_codeword_offsets;
    u8 m_max_length_index;
};

}
