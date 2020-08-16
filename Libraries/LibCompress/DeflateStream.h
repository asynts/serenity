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

#include <AK/InputBitStream.h>
#include <AK/LogStream.h>
#include <AK/Stream.h>

namespace Compress {

class InputDeflateStream final : public InputStream {
public:
    InputDeflateStream(AK::InputBitStream&& stream)
        : m_stream(move(stream))
    {
    }
    InputDeflateStream(AK::InputStream& stream)
        : m_stream(stream)
    {
    }

    size_t read(Bytes) override;
    bool read_or_error(Bytes) override;
    bool eof() const override;
    bool discard_or_error(size_t count) override;

private:
    enum class BlockType {
        Uncompressed = 0b00,
        FixedCompression = 0b01,
        DynamicCompression = 0b10,
        Reserved = 0b11
    };

    struct Block {
        BlockType type;
        bool is_open { false };
        bool is_final_block { false };
    };

    void open_block()
    {
        ASSERT(!m_block.is_final_block);

        m_block.is_open = true;
        m_block.is_final_block = m_stream.read_bits(1);
        m_block.type = static_cast<BlockType>(m_stream.read_bits(2));
        m_stream.discard_or_error(1);

        switch (m_block.type) {
        case BlockType::Uncompressed:
        case BlockType::FixedCompression:
        case BlockType::DynamicCompression:
            break;
        case BlockType::Reserved:
            dbg() << "Invalid deflate block, reserved type.";
            m_error = true;
            break;
        default:
            dbg() << "Invalid deflate block, unknown type.";
            m_error = true;
        }
    }

    void close_block()
    {
        m_block.is_open = false;
    }

    AK::InputBitStream m_stream;
    Block m_block;
};

}
