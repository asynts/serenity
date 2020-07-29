#include "Stream.h"

#include <AK/ByteBuffer.h>
#include <AK/FlyString.h>
#include <AK/String.h>
#include <AK/StringView.h>

#include <cstdio>

OutputStream& OutputStream::operator<<(const String& value)
{
    return *this << value.bytes();
}

OutputStream& OutputStream::operator<<(const StringView& value)
{
    return *this << value.bytes();
}

OutputStream& OutputStream::operator<<(const ByteBuffer& value)
{
    return *this << value.span();
}

OutputStream& OutputStream::operator<<(const FlyString& value)
{
    return *this << value.bytes();
}

OutputStream& OutputStream::operator<<(const void* value)
{
    char buffer[32];
    sprintf(buffer, "%p", value);
    return *this << buffer;
}
