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

#pragma once

#include <AK/CircularQueue.h>
#include <AK/Span.h>
#include <AK/Stream.h>
#include <AK/Types.h>
#include <AK/Vector.h>
#include <cstring>

namespace Compress {

// Reads one bit at a time starting with the rightmost bit
class BitStreamReader {
public:
    BitStreamReader(ReadonlyBytes data)
        : m_data(data)
    {
    }

    i8 read();
    i8 read_byte();
    u32 read_bits(u8);
    u8 get_bit_byte_offset();

private:
    ReadonlyBytes m_data;
    size_t m_data_index { 0 };

    i8 m_current_byte { 0 };
    u8 m_remaining_bits { 0 };
};

class CanonicalCode {
public:
    CanonicalCode(Vector<u8>);
    u32 next_symbol(BitStreamReader&);

private:
    Vector<u32> m_symbol_codes;
    Vector<u32> m_symbol_values;
};

class Deflate {
public:
    Deflate(ReadonlyBytes data)
        : m_reader(data)
        , m_literal_length_codes(generate_literal_length_codes())
        , m_fixed_distance_codes(generate_fixed_distance_codes())
    {
    }

    Vector<u8> decompress();

private:
    void decompress_uncompressed_block();
    void decompress_static_block();
    void decompress_dynamic_block();
    void decompress_huffman_block(CanonicalCode&, CanonicalCode*);
    Vector<CanonicalCode> decode_huffman_codes();
    void copy_from_history(u32, u32);
    u32 decode_run_length(u32);
    u32 decode_distance(u32);
    Vector<u8> generate_literal_length_codes();
    Vector<u8> generate_fixed_distance_codes();

    BitStreamReader m_reader;
    CircularQueue<u8, 32 * 1024> m_history_buffer;
    Vector<u8, 256> m_output_buffer;

    CanonicalCode m_literal_length_codes;
    CanonicalCode m_fixed_distance_codes;
};

// Implements a DEFLATE decompressor according to RFC 1951.
class DeflateStream final
    : public InputStream
    , public Deflate {
public:
    size_t read(Bytes bytes) override
    {
        if (!m_intermediate_stream.eof())
            return m_intermediate_stream.read(bytes);

        if (read_next_block())
            return m_intermediate_stream.read(bytes);

        return 0;
    }

    bool read_or_error(Bytes bytes) override
    {
        if (m_intermediate_stream.remaining() >= bytes.size()) {
            read(bytes);
            return true;
        }

        while (read_next_block()) {
            if (m_intermediate_stream.remaining() >= bytes.size()) {
                read(bytes);
                return true;
            }
        }

        m_error = true;
        return false;
    }

    bool eof() const override
    {
        if (!m_intermediate_stream.eof())
            return false;

        while (read_next_block()) {
            if (!m_intermediate_stream.eof())
                return false;
        }

        return true;
    }

    bool discard_or_error(size_t count) override
    {
        if (m_intermediate_stream.remaining() >= count) {
            m_intermediate_stream.discard_or_error(count);
            return true;
        }

        while (read_next_block()) {
            if (m_intermediate_stream.remaining() >= count) {
                m_intermediate_stream.discard_or_error(count);
                return true;
            }
        }

        m_error = true;
        return false;
    }

private:
    // FIXME: Theoretically, blocks can be extremly large, reading a single block could
    //        exhaust memory. But that's not trivial to implement and could really benefit
    //        from C++20 generators (which don't really have compiler support yet.)
    bool read_next_block() const
    {
        if (m_read_last_block)
            return false;

        TODO();
    }

    mutable bool m_read_last_block { false };
    mutable DuplexMemoryStream m_intermediate_stream;
};

}
