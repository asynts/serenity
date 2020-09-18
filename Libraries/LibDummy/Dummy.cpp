#include <AK/Format.h>

String format_using_new_format(u32 value);
String format_using_new_format(u32 value)
{
    return AK::format("{}", value);
}

String format_using_printf(u32 value);
String format_using_printf(u32 value)
{
    return String::format("%u", value);
}
