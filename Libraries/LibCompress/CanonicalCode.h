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
#include <AK/Optional.h>
#include <AK/StdLibExtras.h>

namespace Compress::New {

class Reader {
public:
    // Padded with zeroes if not enough bits are avaliable.
    u16 peek() const;

    void consume(size_t bits);
};

// This is very much inspired by "Efficient Huffman Decoding" in https://www.hanshq.net/zip.html.
class CanonicalCode {
public:
    static constexpr u16 no_symbol_found = 0xff;

    u16 read_symbol()
    {
        const auto bits = m_reader.peek();

        for (size_t idx = 0; idx < min<size_t>(m_bits_used, 15); ++idx) {
            if (bits < m_sentinel_bits[idx]) {
                m_reader.consume(idx + 1);
                return m_first_codeword_offsets[idx] + bits & (1u << idx - 1);
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
