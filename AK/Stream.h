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

// FIXME: Can we somehow remove these dependencies?
#include <AK/String.h>
#include <AK/StringView.h>

namespace AK {

namespace Detail {

class Stream {
public:
    inline bool has_error() const
    {
        return !m_error.is_null();
    }

    inline StringView error() const
    {
        return m_error;
    }

    inline String handle_error()
    {
        return exchange(m_error, {});
    }

protected:
    String m_error;
};

}

class InputStream : public virtual Detail::Stream {
public:
    virtual ~InputStream() = 0;

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

    virtual void write(ReadonlyBytes) = 0;

    template<typename T, typename = typename EnableIf<IsIntegral<T>::value>::Type>
    OutputStream& operator<<(T value)
    {
        write({ &value, sizeof(value) });
        return *this;
    }

    inline OutputStream& operator<<(bool value)
    {
        return *this << (value ? "true" : "false");
    }

    inline OutputStream& operator<<(const char* value)
    {
        if (!value)
            return *this << "(null)";

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
    OutputStream& operator<<(const void*);

#ifndef KERNEL
    template<typename T, typename = typename EnableIf<IsFloatingPoint<T>::value>::Type>
    inline OutputStream& operator<<(T value)
    {
        write({ &value, sizeof(value) });
        return *this;
    }
#endif
};

class DuplexStream
    : public InputStream
    , public OutputStream {
public:
    virtual ~DuplexStream() = 0;
};

}
