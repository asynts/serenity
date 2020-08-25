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
#include <LibCompress/Endian.h>

namespace Compress {

class CanonicalCode {
public:
    CanonicalCode() = default;
    CanonicalCode(ReadonlyBytes);
    u32 read_symbol(InputBitStream&) const;

    static const CanonicalCode& fixed_literal_codes() { TODO(); }
    static const CanonicalCode& fixed_distance_codes() { TODO(); }

private:
    Vector<u32> m_symbol_codes;
    Vector<u32> m_symbol_values;
};

class DeflateDecompressor;

class CompressedBlock {
public:
    CompressedBlock(DeflateDecompressor& decompressor, CanonicalCode literal_codes, Optional<CanonicalCode> distance_codes)
        : m_decompressor(decompressor)
        , m_literal_codes(literal_codes)
        , m_distance_codes(distance_codes)
    {
    }

    bool try_read_more();

private:
    bool m_eof { false };

    DeflateDecompressor& m_decompressor;
    CanonicalCode m_literal_codes;
    Optional<CanonicalCode> m_distance_codes;
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
        if (m_state == State::Ready) {
            if (m_read_final_bock)
                return 0;

            m_read_final_bock = m_input_stream.read_bit();
            const auto block_type = m_input_stream.read_bits(2);

            if (block_type == 0b00) {
                m_input_stream.align_to_byte_boundary();

                LittleEndian<u16> length, negated_length;
                m_input_stream >> length >> negated_length;

                if ((length ^ 0xffff) != negated_length) {
                    m_error = true;
                    return 0;
                }

                m_state = State::ReadingUncompressedBlock;
                new (&m_uncompressed_block) UncompressedBlock(*this, length);

                return read(bytes);
            }

            if (block_type == 0b01) {
                m_state = State::ReadingCompressedBlock;
                new (&m_compressed_block) CompressedBlock(*this, CanonicalCode::fixed_literal_codes(), CanonicalCode::fixed_distance_codes());

                return read(bytes);
            }

            if (block_type == 0b10) {
                CanonicalCode literal_codes, distance_codes;
                decode_codes(literal_codes, distance_codes);
                new (&m_compressed_block) CompressedBlock(*this, literal_codes, distance_codes);

                return read(bytes);
            }

            ASSERT_NOT_REACHED();
        }

        if (m_state == State::ReadingCompressedBlock) {
            auto nread = m_output_stream.read(bytes);

            while (nread < bytes.size() && m_compressed_block.try_read_more()) {
                nread += m_output_stream.read(bytes.slice(nread));
            }

            if (nread == bytes.size())
                return nread;

            m_compressed_block.~CompressedBlock();
            m_state = State::Ready;

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
            m_state = State::Ready;

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

    bool discard_or_error(size_t count) override
    {
        u8 buffer[4096];

        size_t ndiscarded = 0;
        while (ndiscarded < count) {
            if (eof()) {
                m_error = true;
                return false;
            }

            ndiscarded += read({ buffer, min<size_t>(count - ndiscarded, 4096) });
        }

        return true;
    }

    bool eof() const override { return m_state == State::Ready && m_read_final_bock; }

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
        Ready,
        ReadingCompressedBlock,
        ReadingUncompressedBlock
    };

    u32 decode_run_length(u32);
    u32 decode_distance(u32);
    void decode_codes(CanonicalCode&, CanonicalCode&);

    bool m_read_final_bock { false };

    State m_state { State::Ready };
    union {
        CompressedBlock m_compressed_block;
        UncompressedBlock m_uncompressed_block;
    };

    InputBitStream m_input_stream;
    CircularDuplexStream<64 * 1024> m_output_stream;

    friend CompressedBlock;
    friend UncompressedBlock;
};

}
