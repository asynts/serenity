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

#include <AK/Stream.h>

namespace Compress {

class DeflateCompressor final : public OutputStream {
public:
    explicit DeflateCompressor(OutputStream& stream)
        : m_stream(stream)
    {
        allocate_buffer();
    }

    ~DeflateCompressor()
    {
        flush_buffer();
        deallocate_buffer();
    }

    size_t write(ReadonlyBytes bytes) override
    {
        if (has_any_error())
            return 0;

        auto nwritten = bytes.copy_trimmed_to(m_buffer.slice(m_buffer_used));
        m_buffer_used += nwritten;

        if (bytes.size() - nwritten > 0) {
            flush_buffer();
            nwritten += write(bytes.slice(nwritten));
        }

        return nwritten;
    }

    bool write_or_error(ReadonlyBytes bytes)
    {
        if (write(bytes) != bytes.size()) {
            set_fatal_error();
            return false;
        }

        return true;
    }

private:
    void allocate_buffer();
    void deallocate_buffer();
    void flush_buffer();

    OutputStream& m_stream;
    Bytes m_buffer;
    size_t m_buffer_used = 0;
};

}
