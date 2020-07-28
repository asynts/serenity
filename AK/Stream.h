#pragma once

#include <AK/Span.h>
#include <AK/StdLibExtras.h>
#include <AK/String.h>

#include <string.h>

namespace AK {

namespace Detail {

class Stream {
public:
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

class IStream : public Detail::Stream {
public:
    virtual ~IStream() = 0;

    virtual size_t read(Bytes) = 0;

    // FIXME: Make this generic for all primitive types.
    inline IStream& operator>>(int& value)
    {
        read({ &value, sizeof(int) });
        return *this;
    }
};

class OStream : public Detail::Stream {
public:
    virtual ~OStream() = 0;

    virtual size_t write(ReadonlyBytes) = 0;

    // FIXME: Make this generic for all primitive types.
    inline OStream& operator<<(int value)
    {
        write({ &value, sizeof(value) });
        return *this;
    }

    inline OStream& operator<<(const char* value)
    {
        write({ value, strlen(value) });
        return *this;
    }
};

// FIXME: Do we have that multi-inheritence problem?
class IOStream
    : public IStream
    , public OStream {
public:
    virtual ~IOStream() = 0;
};

}
