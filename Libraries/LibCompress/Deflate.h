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

#include <AK/ByteBuffer.h>
#include <LibCompress/CircularDuplexStream.h>

namespace Compress {

class CanonicalCode {
public:
    CanonicalCode(ReadonlyBytes);
    u32 read_symbol(InputBitStream&) const;

private:
    Vector<u32> m_symbol_codes;
    Vector<u32> m_symbol_values;
};

class DeflateDecompressor;

class CompressedBlock {
public:
    CompressedBlock(DeflateDecompressor& decompressor, const CanonicalCode& length_codes, const CanonicalCode* distance_codes)
        : m_decompressor(decompressor)
        , m_length_codes(length_codes)
        , m_distance_codes(distance_codes)
    {
    }

    bool try_read_more();

private:
    bool m_eof { false };

    DeflateDecompressor& m_decompressor;
    const CanonicalCode& m_length_codes;
    const CanonicalCode* m_distance_codes;
};

class UncompressedBlock {
public:
    UncompressedBlock(DeflateDecompressor& decompressor, size_t length)
        : m_decompressor(decompressor)
        , m_bytes_remaining(length)
    {
    }

    bool try_read_more();

private:
    DeflateDecompressor& m_decompressor;
    size_t m_bytes_remaining;
};

class DeflateDecompressor : public InputStream {
public:
    DeflateDecompressor(InputStream& stream)
        : m_input_stream(stream)
        , m_literal_length_codes(generate_literal_length_codes())
        , m_fixed_distance_codes(generate_fixed_distance_codes())
    {
    }

    ~DeflateDecompressor()
    {
        if (m_state == State::ReadingCompressedBlock)
            m_compressed_block.~CompressedBlock();
        if (m_state == State::ReadingUncompressedBlock)
            m_uncompressed_block.~UncompressedBlock();
    }

    size_t read(Bytes bytes) override
    {
        if (m_state == State::Idle) {
            TODO();
        }

        if (m_state == State::ReadingCompressedBlock) {
            auto nread = m_output_stream.read(bytes);

            while (nread < bytes.size() && m_compressed_block.try_read_more()) {
                nread += m_output_stream.read(bytes.slice(nread));
            }

            if (nread == bytes.size())
                return nread;

            m_compressed_block.~CompressedBlock();
            m_state = State::Idle;

            return nread + read(bytes.slice(nread));
        }

        if (m_state == State::ReadingUncompressedBlock) {
            auto nread = m_output_stream.read(bytes);

            while (nread < bytes.size() && m_uncompressed_block.try_read_more()) {
                nread += m_output_stream.read(bytes.slice(nread));
            }

            if (nread == bytes.size())
                return nread;

            m_uncompressed_block.~UncompressedBlock();
            m_state = State::Idle;

            return nread + read(bytes.slice(nread));
        }

        ASSERT_NOT_REACHED();
    }

    bool read_or_error(Bytes bytes) override
    {
        if (read(bytes) < bytes.size()) {
            m_error = true;
            return false;
        }

        return true;
    }

    bool discard_or_error(size_t) override { TODO(); }
    bool eof() const override { TODO(); }

    static ByteBuffer decompress_all(ReadonlyBytes bytes)
    {
        InputMemoryStream memory_stream { bytes };
        InputBitStream bit_stream { memory_stream };
        DeflateDecompressor deflate_stream { bit_stream };

        auto buffer = ByteBuffer::create_uninitialized(4096);
        size_t nread = 0;

        while (!deflate_stream.eof()) {
            nread += deflate_stream.read(buffer.bytes().slice(nread));
            if (buffer.size() - nread < 4096)
                buffer.grow(buffer.size() + 4096);
        }

        buffer.trim(nread);
        return buffer;
    }

private:
    enum class State {
        Idle,
        ReadingCompressedBlock,
        ReadingUncompressedBlock
    };

    u32 decode_run_length(u32);
    u32 decode_distance(u32);

    Vector<u8> generate_literal_length_codes();
    Vector<u8> generate_fixed_distance_codes();

    State m_state { State::Idle };
    union {
        CompressedBlock m_compressed_block;
        UncompressedBlock m_uncompressed_block;
    };

    InputBitStream m_input_stream;
    CircularDuplexStream<64 * 1024> m_output_stream;

    CanonicalCode m_literal_length_codes;
    CanonicalCode m_fixed_distance_codes;

    friend CompressedBlock;
    friend UncompressedBlock;
};

}
