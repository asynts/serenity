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

class IStream : public virtual Detail::Stream {
public:
    virtual ~IStream() = 0;

    virtual size_t read(Bytes) = 0;

    template<typename T, typename = typename EnableIf<IsIntegral<T>::value>::Type>
    inline IStream& operator>>(T& value)
    {
        read({ &value, sizeof(value) });
        return *this;
    }
};

class OStream : public virtual Detail::Stream {
public:
    virtual ~OStream() = 0;

    virtual void write(ReadonlyBytes) = 0;

    template<typename T, typename = typename EnableIf<IsIntegral<T>::value>::Type>
    inline OStream& operator<<(T value)
    {
        write({ &value, sizeof(value) });
        return *this;
    }

    inline OStream& operator<<(const char* value)
    {
        write({ value, __builtin_strlen(value) });
        return *this;
    }

    inline OStream& operator<<(ReadonlyBytes bytes)
    {
        write(bytes);
        return *this;
    }

    OStream& operator<<(const ByteBuffer&);
    OStream& operator<<(const String&);
    OStream& operator<<(const StringView&);
};

class IOStream
    : public IStream
    , public OStream {
public:
    virtual ~IOStream() = 0;
};

}
