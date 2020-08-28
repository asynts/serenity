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

#include <LibCompress/Deflate.h>

namespace Compress {

class GzipDecompressor final : public InputStream {
public:
    GzipDecompressor(InputStream&);
    ~GzipDecompressor();

    size_t read(Bytes) override;
    bool read_or_error(Bytes) override;
    bool discard_or_error(size_t) override;
    bool eof() const override;

    static ByteBuffer decompress_all(ReadonlyBytes);

private:
    struct [[gnu::packed]] BlockHeader
    {
        u8 identification_1;
        u8 identification_2;
        u8 compression_method;
        u8 flags;
        LittleEndian<u32> modification_time;
        u8 extra_flags;
        u8 operating_system;

        bool valid_magic_number() const;
        bool supported_by_implementation() const;
    };

    struct Flags {
        static constexpr u8 FTEXT = 1 << 0;
        static constexpr u8 FHCRC = 1 << 1;
        static constexpr u8 FEXTRA = 1 << 1;
        static constexpr u8 FNAME = 1 << 1;
        static constexpr u8 FCOMMENT = 1 << 1;

        static constexpr u8 MAX = FTEXT | FHCRC | FEXTRA | FNAME | FCOMMENT;
    };

    enum class State {
        Idle,
        ReadingDeflateBlock
    };

    State m_state { State::Idle };
    union {
        DeflateDecompressor m_block_stream;
        BlockHeader m_block_header;
    };

    InputStream& m_input_stream;
};

}
