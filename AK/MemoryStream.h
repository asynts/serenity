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

namespace AK {

class InputMemoryStream final : public InputStream {
public:
    inline explicit InputMemoryStream(ReadonlyBytes bytes)
        : m_bytes(bytes)
    {
    }

    inline ~InputMemoryStream()
    {
        ASSERT(!error());
    }

    inline size_t read(Bytes bytes) override
    {
        const auto count = min(m_bytes.size() - m_offset, bytes.size());
        __builtin_memcpy(bytes.data(), m_bytes.data() + m_offset, count);
        m_offset += count;
        return count;
    }

    inline void seek(size_t offset)
    {
        ASSERT(offset < m_bytes.size());
        m_offset = offset;
    }

    inline size_t offset() const { return m_offset; }

private:
    ReadonlyBytes m_bytes;
    size_t m_offset { 0 };
};

class OutputMemoryStream final : public OutputStream {
public:
    inline explicit OutputMemoryStream(Bytes bytes)
        : m_bytes(bytes)
    {
    }

    inline ~OutputMemoryStream()
    {
        ASSERT(!error());
    }

    inline void write(ReadonlyBytes bytes) override
    {
        if (bytes.size() > m_bytes.size() - m_offset) {
            m_error = true;
            return;
        }

        __builtin_memcpy(m_bytes.data() + m_offset, bytes.data(), bytes.size());
        m_offset += bytes.size();
    }

    inline void seek(size_t offset)
    {
        ASSERT(offset < m_bytes.size());
        m_offset = offset;
    }

    inline void fill_until_end(u8 fill)
    {
        __builtin_memset(m_bytes.data() + m_offset, fill, m_bytes.size() - m_offset);
        m_offset = m_bytes.size();
    }

    inline size_t offset() const { return m_offset; }

private:
    Bytes m_bytes;
    size_t m_offset { 0 };
};

}

using AK::InputMemoryStream;
using AK::OutputMemoryStream;
