/*
 * Copyright (c) 2020, the SerenityOS developers
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

#include <AK/Assertions.h>
#include <AK/LogStream.h>
#include <AK/Span.h>
#include <AK/Types.h>
#include <AK/Vector.h>
#include <LibCompress/Deflate.h>

#include <cstring>

namespace Compress {

bool CompressedBlock::try_read_more()
{
    if (m_eof == true)
        return false;

    const auto symbol = m_literal_codes.read_symbol(m_decompressor.m_input_stream);

    if (symbol < 256) {
        m_decompressor.m_output_stream << static_cast<u8>(symbol);
        return true;
    } else if (symbol == 256) {
        m_eof = true;
        return false;
    } else {
        ASSERT(m_distance_codes.has_value());

        const auto run_length = m_decompressor.decode_run_length(symbol);
        const auto distance = m_decompressor.decode_distance(m_distance_codes.value().read_symbol(m_decompressor.m_input_stream));

        auto bytes = m_decompressor.m_output_stream.reserve_contigous(run_length);
        m_decompressor.m_output_stream.read(bytes, distance);

        return true;
    }
}

bool UncompressedBlock::try_read_more()
{
    if (m_bytes_remaining == 0)
        return false;

    const auto nread = min(m_bytes_remaining, m_decompressor.m_output_stream.remaining_contigous_space());
    m_bytes_remaining -= nread;

    m_decompressor.m_input_stream >> m_decompressor.m_output_stream.reserve_contigous(nread);

    return true;
}

u32 DeflateDecompressor::decode_run_length(u32 symbol)
{
    if (symbol <= 264)
        return symbol - 254;

    if (symbol <= 284) {
        auto extra_bits = (symbol - 261) / 4;
        return (((symbol - 265) % 4 + 4) << extra_bits) + 3 + m_input_stream.read_bits(extra_bits);
    }

    if (symbol == 285)
        return 258;

    ASSERT_NOT_REACHED();
}

u32 DeflateDecompressor::decode_distance(u32 symbol)
{
    if (symbol <= 3)
        return symbol + 1;

    if (symbol <= 29) {
        auto extra_bits = (symbol / 2) - 1;
        return ((symbol % 2 + 2) << extra_bits) + 1 + m_input_stream.read_bits(extra_bits);
    }

    ASSERT_NOT_REACHED();
}

void DeflateDecompressor::decode_codes(CanonicalCode&, CanonicalCode&) { TODO(); }

CanonicalCode::CanonicalCode(ReadonlyBytes codes)
{
    m_symbol_codes.resize(codes.size());
    m_symbol_values.resize(codes.size());

    auto allocated_symbols_count = 0;
    auto next_code = 0;

    for (size_t code_length = 1; code_length <= 15; code_length++) {
        next_code <<= 1;
        auto start_bit = 1 << code_length;

        for (size_t symbol = 0; symbol < codes.size(); symbol++) {
            if (codes.at(symbol) != code_length) {
                continue;
            }

            if (next_code > start_bit) {
                dbg() << "Canonical code overflows the huffman tree";
                ASSERT_NOT_REACHED();
            }

            m_symbol_codes[allocated_symbols_count] = start_bit | next_code;
            m_symbol_values[allocated_symbols_count] = symbol;

            allocated_symbols_count++;
            next_code++;
        }
    }

    if (next_code != (1 << 15)) {
        dbg() << "Canonical code underflows the huffman tree " << next_code;
        ASSERT_NOT_REACHED();
    }
}

static i32 binary_search(const Vector<u32>& haystack, u32 needle)
{
    i32 low = 0;
    i32 high = haystack.size();

    while (low <= high) {
        u32 mid = (low + high) >> 1;
        u32 value = haystack.at(mid);

        if (value < needle) {
            low = mid + 1;
        } else if (value > needle) {
            high = mid - 1;
        } else {
            return mid;
        }
    }

    return -1;
}

u32 CanonicalCode::read_symbol(InputBitStream& stream) const
{
    u32 code_bits = 1;

    for (;;) {
        code_bits = code_bits << 1 | stream.read_bits(1);
        const auto index = binary_search(m_symbol_codes, code_bits);
        if (index >= 0)
            return m_symbol_values[index];
    }
}

}
