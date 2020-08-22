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

#include <AK/BitStream.h>

namespace Compress {

class DeflateDecompressor final : public InputStream {
public:
    DeflateDecompressor(InputStream& stream)
        : m_stream(stream)
    {
    }

    ~DeflateDecompressor()
    {
        switch (m_status) {
        case Status::Idle:
            return;
        case Status::ReadingUncompressedBlock:
            m_uncompressed_block_stream.~UncompressedBlockStream();
            return;
        case Status::ReadingCompressedBlock:
            m_compressed_block_stream.~CompressedBlockStream();
            return;
        default:
            ASSERT_NOT_REACHED();
        }
    }

    size_t read(Bytes bytes) override
    {
        if (m_status == Status::Idle) {
            if (m_is_final_block)
                return 0;

            m_is_final_block = m_stream.read_bit();
            const auto block_type = m_stream.read_bits(2);

            if (m_stream.handle_error()) {
                m_error = true;
                return 0;
            }

            switch (block_type) {
            case 0b00:
                new (&m_uncompressed_block_stream) UncompressedBlockStream(m_stream);
                m_status = Status::ReadingUncompressedBlock;
                return read(bytes);
            case 0b01:
                new (&m_compressed_block_stream) CompressedBlockStream(fixed_huffman_codes);
                m_status = Status::ReadingCompressedBlock;
                return read(bytes);
            case 0b10:
                new (&m_compressed_block_stream) CompressedBlockStream(decode_huffman_codes());
                m_status = Status::ReadingCompressedBlock;
                return read(bytes);

            default:
                dbg() << "Invalid block type " << block_type;
                m_error = true;
                return 0;
            }
        } else if (m_status == Status::ReadingUncompressedBlock) {
            TODO();
        } else if (m_status == Status::ReadingCompressedBlock) {
            TODO();
        } else {
            ASSERT_NOT_REACHED();
        }
    }

    bool read_or_error(Bytes bytes) override
    {
        if (read(bytes) != bytes.size()) {
            m_error = true;
            return false;
        }

        return true;
    }

    bool eof() const override
    {
        if (!m_is_final_block)
            return false;

        switch (m_status) {
        case Status::Idle:
            return true;
        case Status::ReadingUncompressedBlock:
            return m_uncompressed_block_stream.eof();
        case Status::ReadingCompressedBlock:
            return m_compressed_block_stream.eof();
        default:
            ASSERT_NOT_REACHED();
        }
    }

    bool discard_or_error(size_t) { TODO(); }

    class HuffmanCodes {
    public:
        HuffmanCodes() { TODO(); }
    };

    static const HuffmanCodes fixed_huffman_codes;

private:
    enum class Status {
        Idle,
        ReadingUncompressedBlock,
        ReadingCompressedBlock
    };

    HuffmanCodes decode_huffman_codes() { TODO(); }

    class UncompressedBlockStream final : public InputStream {
    public:
        UncompressedBlockStream(InputBitStream<>& stream)
            : m_stream(stream)
        {
            m_stream.align_to_byte_boundary();

            u16 length, inverse_length;
            m_stream >> length >> inverse_length;

            if (m_stream.handle_error() || (length ^ 0xffff) != inverse_length) {
                m_error = true;
                return;
            }

            m_bytes_left = length;
        }

        size_t read(Bytes bytes) override
        {
            const auto nread = m_stream.read_or_error(bytes.trim(m_bytes_left));

            if (m_stream.handle_error()) {
                m_error = true;
                return false;
            }

            m_bytes_left -= nread;
            return nread;
        }

        bool read_or_error(Bytes bytes) override
        {
            if (m_bytes_left < bytes.size()) {
                m_error = true;
                return false;
            }

            read(bytes);
            return true;
        }

        bool eof() const override { return m_bytes_left == 0; }

        bool discard_or_error(size_t count) override
        {
            if (m_bytes_left < count) {
                m_error = true;
                return false;
            }

            m_stream.discard_or_error(count);

            if (m_stream.handle_error()) {
                m_error = true;
                return false;
            }

            return true;
        }

    private:
        InputBitStream<>& m_stream;
        size_t m_bytes_left { 0 };
    };

    class CompressedBlockStream final : public InputStream {
    public:
        CompressedBlockStream(HuffmanCodes) { TODO(); }
        size_t read(Bytes) override { TODO(); }
        bool read_or_error(Bytes) { TODO(); }
        bool eof() const { TODO(); }
        bool discard_or_error(size_t) { TODO(); }
    };

    InputBitStream<> m_stream;
    bool m_is_final_block { false };

    Status m_status { Status::Idle };
    union {
        UncompressedBlockStream m_uncompressed_block_stream;
        CompressedBlockStream m_compressed_block_stream;
    };
};
}
