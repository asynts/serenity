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

#include <AK/Span.h>
#include <AK/StdLibExtras.h>

namespace AK {

namespace Detail {

class Stream {
public:
    virtual ~Stream() = 0;

    inline bool error() const { return m_error; }

    inline bool handle_error() { return exchange(m_error, false); }

    inline operator bool() const { return !m_error; }

protected:
    bool m_error { false };
};
inline Stream::~Stream() { }

}

class InputStream : public virtual Detail::Stream {
public:
    virtual ~InputStream() = 0;

    virtual size_t read(Bytes) = 0;

    inline bool read_or_error(Bytes bytes)
    {
        if (read(bytes) < bytes.size()) {
            m_error = true;
            return false;
        }

        return true;
    }

    template<typename Callback>
    void for_each_chunk(Callback, size_t recommended_chunk_size = 1024);
};
inline InputStream::~InputStream() { }

class OutputStream : public virtual Detail::Stream {
public:
    virtual ~OutputStream() = 0;

    virtual void write(ReadonlyBytes) = 0;
};
inline OutputStream::~OutputStream() { }

class DuplexStream
    : public InputStream
    , public OutputStream {
public:
    virtual ~DuplexStream() = 0;
};
inline DuplexStream::~DuplexStream() { }

class NullStream final : public DuplexStream {
    void write(ReadonlyBytes) override { }

    size_t read(Bytes bytes) override
    {
        __builtin_memset(bytes.data(), 0, bytes.size());
        return bytes.size();
    }
};

#ifdef KERNEL
// FIXME: Provide overloads for all integer types.
inline InputStream& operator>>(InputStream& stream, u32& value)
{
    stream.read_or_error({ &value, sizeof(value) });
    return stream;
}
// FIXME: Provide overloads for all integer types.
inline OutputStream& operator<<(OutputStream& stream, u32 value)
{
    stream.write({ &value, sizeof(value) });
    return stream;
}
#else
// FIXME: Provide overloads for all integer types.
inline InputStream& operator>>(InputStream& stream, u32& value)
{
    stream.read_or_error({ &value, sizeof(value) });
    return stream;
}
// FIXME: Provide overloads for all integer types.
inline OutputStream& operator<<(OutputStream& stream, u32 value)
{
    stream.write({ &value, sizeof(value) });
    return stream;
}

// FIXME: Provide overloads for floating point numbers.
#endif

inline InputStream& operator>>(InputStream& stream, bool& value)
{
    stream.read_or_error({ &value, sizeof(value) });
    return stream;
}
inline OutputStream& operator<<(OutputStream& stream, bool value)
{
    stream.write({ &value, sizeof(value) });
    return stream;
}

inline InputStream& operator>>(InputStream& stream, Bytes bytes)
{
    stream.read_or_error(bytes);
    return stream;
}
inline OutputStream& operator<<(OutputStream& stream, ReadonlyBytes bytes)
{
    stream.write(bytes);
    return stream;
}

inline InputStream& operator>>(InputStream& lhs, OutputStream& rhs)
{
    u8 buffer[1024];
    size_t count = 0;
    while ((count = lhs.read({ buffer, sizeof(buffer) }))) {
        rhs.write({ buffer, count });
    }

    return lhs;
}
inline OutputStream& operator<<(OutputStream& lhs, InputStream& rhs)
{
    u8 buffer[1024];
    size_t count = 0;
    while ((count = rhs.read({ buffer, sizeof(buffer) }))) {
        lhs.write({ buffer, count });
    }

    return lhs;
}

inline OutputStream& operator<<(OutputStream& stream, const char* value)
{
    stream.write({ value, __builtin_strlen(value) });
    return stream;
}

template<typename Callback>
void InputStream::for_each_chunk(Callback callback, size_t recommended_chunk_size)
{
    u8 buffer[recommended_chunk_size];
    Bytes bytes { buffer, sizeof(buffer) };
    while (*this >> bytes) {
        if (callback(bytes) == IterationDecision::Break)
            return;
    }

    handle_error();

    const auto size = read(bytes);
    callback(bytes.slice(size));
}

}

using AK::DuplexStream;
using AK::InputStream;
using AK::OutputStream;
