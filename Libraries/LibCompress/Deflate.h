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

#include <AK/BitStream.h>
#include <AK/CircularQueue.h>
#include <AK/Span.h>
#include <AK/Types.h>
#include <AK/Vector.h>

namespace Compress {

class CanonicalCode {
public:
    CanonicalCode(ReadonlyBytes);
    u32 read_symbol(InputBitStream&) const;

private:
    Vector<u32> m_symbol_codes;
    Vector<u32> m_symbol_values;
};

class Deflate;

class CompressedBlock {
public:
    CompressedBlock(Deflate& decompressor, const CanonicalCode& length_codes, const CanonicalCode* distance_codes)
        : m_decompressor(decompressor)
        , m_length_codes(length_codes)
        , m_distance_codes(distance_codes)
    {
    }

    bool try_read_more();

private:
    bool m_eof { false };

    Deflate& m_decompressor;
    const CanonicalCode& m_length_codes;
    const CanonicalCode* m_distance_codes;
};

class UncompressedBlock {
public:
    UncompressedBlock(Deflate& decompressor, size_t length)
        : m_decompressor(decompressor)
        , m_bytes_remaining(length)
    {
    }

    bool try_read_more();

private:
    Deflate& m_decompressor;
    size_t m_bytes_remaining;
};

class Deflate {
public:
    Deflate(InputStream& stream)
        : m_input_stream(stream)
        , m_literal_length_codes(generate_literal_length_codes())
        , m_fixed_distance_codes(generate_fixed_distance_codes())
    {
    }

    ~Deflate()
    {
        if (m_state == State::ReadingCompressedBlock)
            m_huffman_block.~CompressedBlock();
    }

    Vector<u8> decompress();

    static Vector<u8> decompress_all(ReadonlyBytes bytes)
    {
        InputMemoryStream memory_stream { bytes };
        Deflate deflate_stream { memory_stream };
        return deflate_stream.decompress();
    }

private:
    enum class State {
        Idle,
        ReadingCompressedBlock,
        ReadingUncompressedBlock
    };

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

    State m_state { State::Idle };
    union {
        CompressedBlock m_huffman_block;
    };

    InputBitStream m_input_stream;

    CircularQueue<u8, 32 * 1024> m_history_buffer;
    Vector<u8, 256> m_output_buffer;

    CanonicalCode m_literal_length_codes;
    CanonicalCode m_fixed_distance_codes;

    friend CompressedBlock;
    friend UncompressedBlock;
};

}
