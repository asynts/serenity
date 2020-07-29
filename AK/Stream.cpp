#include "Stream.h"

#include <AK/ByteBuffer.h>
#include <AK/String.h>
#include <AK/StringView.h>

OStream& OStream::operator<<(const String& value)
{
    return *this << value.bytes();
}

OStream& OStream::operator<<(const StringView& value)
{
    return *this << value.bytes();
}

OStream& OStream::operator<<(const ByteBuffer& value)
{
    return *this << value.span();
}
