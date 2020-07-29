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

// Note that GCC can de-virtualize the calls to IStream::read() and OStream::write()
// if -O2 is enabled. (https://godbolt.org/z/Mcb9bq)

namespace Detail {

class Stream {
public:
    inline bool error() const { return m_error; }

    inline bool handle_error() { return exchange(m_error, false); }

protected:
    bool m_error;
};

}

class InputStream : public virtual Detail::Stream {
public:
    virtual ~InputStream() = 0;

    // FIXME: This function will be called very frequently, can we help
    //        the compiler de-virtualize this call?
    virtual size_t read(Bytes) = 0;

    template<typename T, typename = typename EnableIf<IsIntegral<T>::value>::Type>
    InputStream& operator>>(T& value)
    {
        read({ &value, sizeof(value) });
        return *this;
    }

    inline InputStream& operator>>(bool& value)
    {
        read({ &value, sizeof(value) });
        return *this;
    }
};

class OutputStream : public virtual Detail::Stream {
public:
    virtual ~OutputStream() = 0;

    // FIXME: This function will be called very frequently, can we help
    //        the compiler de-virtualize this call?
    virtual void write(ReadonlyBytes) = 0;

#ifdef KERNEL
    template<typename T, typename = typename EnableIf<IsIntegral<T>::value>::Type>
    OutputStream& operator<<(T value)
    {
        write({ &value, sizeof(value) });
        return *this;
    }
#else
    template<typename T, typename = typename EnableIf<IsIntegral<T>::value || IsFloatingPoint<T>::value>::Type>
    OutputStream& operator<<(T value)
    {
        write({ &value, sizeof(value) });
        return *this;
    }
#endif

    inline OutputStream& operator<<(bool value)
    {
        write({ &value, sizeof(value) });
        return *this;
    }

    inline OutputStream& operator<<(const char* value)
    {
        write({ value, __builtin_strlen(value) });
        return *this;
    }

    inline OutputStream& operator<<(ReadonlyBytes bytes)
    {
        write(bytes);
        return *this;
    }

    OutputStream& operator<<(const ByteBuffer&);
    OutputStream& operator<<(const String&);
    OutputStream& operator<<(const StringView&);
    OutputStream& operator<<(const FlyString&);
};

class DuplexStream
    : public InputStream
    , public OutputStream {
public:
    virtual ~DuplexStream() = 0;
};

class NullStream final : public DuplexStream {
    void write(ReadonlyBytes) override { }

    size_t read(Bytes bytes) override
    {
        __builtin_memset(bytes.data(), 0, bytes.size());
        return bytes.size();
    }
};

class InputMemoryStream final : public InputStream {
public:
    inline explicit InputMemoryStream(ReadonlyBytes bytes)
        : m_bytes(bytes)
    {
    }

    size_t read(Bytes bytes) override
    {
        const auto count = min(m_bytes.size() - m_offset, bytes.size());
        __builtin_memcpy(bytes.data(), m_bytes.data(), count);
        m_offset += count;
        return count;
    }

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

    void write(ReadonlyBytes bytes) override
    {
        if (bytes.size() > m_bytes.size() - m_offset) {
            m_error = true;
            return;
        }
        __builtin_memcpy(m_bytes.data(), bytes.data(), bytes.size());
        m_offset += bytes.size();
    }

private:
    Bytes m_bytes;
    size_t m_offset { 0 };
};

}
