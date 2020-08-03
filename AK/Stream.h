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

#include <AK/Assertions.h>
#include <AK/Concepts.h>
#include <AK/Span.h>

namespace AK {

class BaseStream {
public:
    ~BaseStream()
    {
        ASSERT(!error() && !fatal());
    }

    inline bool error() const { return m_error; }

    inline bool handle_error()
    {
        return exchange(m_error, false);
    }

    inline bool fatal() const { return m_fatal; }

protected:
    bool m_error { false };
    bool m_fatal { false };
};

template<Concepts::SimpleInputStream Stream, Concepts::Integral T>
Stream& operator>>(Stream& stream, T& value)
{
    stream.read({ &value, sizeof(value) });
    return stream;
}

#ifndef KERNEL
template<Concepts::SimpleInputStream Stream, Concepts::FloatingPoint T>
Stream& operator>>(Stream& stream, T& value)
{
    stream.read({ &value, sizeof(value) });
    return stream;
}
#endif

class InputMemoryStream : public BaseStream {
public:
    inline explicit InputMemoryStream(ReadonlyBytes bytes)
        : m_bytes(bytes)
    {
    }

    inline size_t read(Bytes bytes)
    {
        const auto count = min(bytes.size(), m_bytes.size() - m_offset);

        __builin_memcpy(bytes.data(), m_bytes.data() + m_offset, count);
        m_offset += count;

        return count;
    }

private:
    size_t m_offset { 0 };
    ReadonlyBytes m_bytes;
};

}
