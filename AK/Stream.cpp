#include "Stream.h"

#include <AK/ByteBuffer.h>
#include <AK/String.h>
#include <AK/StringView.h>

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
