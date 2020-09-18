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

#include <AK/Optional.h>
#include <AK/String.h>
#include <AK/StringBuilder.h>
#include <AK/StringView.h>

namespace AK {

template<typename T>
struct Formatter {
    bool parse(StringView);
    void format(StringBuilder&, const T&);
};

} // namespace AK

namespace AK::Detail::Format {

inline bool find_next_unescaped(size_t& index, StringView input, char ch)
{
    constexpr size_t unset = NumericLimits<size_t>::max();

    index = unset;
    for (size_t idx = 0; idx < input.length(); ++idx) {
        if (input[idx] == ch) {
            if (index == unset)
                index = idx;
            else
                index = unset;
        } else if (index != unset) {
            return true;
        }
    }

    return false;
}
inline bool find_next(size_t& index, StringView input, char ch)
{
    for (index = 0; index < input.length(); ++index) {
        if (input[index] == ch)
            return true;
    }

    return false;
}
inline void write_escaped_literal(StringBuilder& builder, StringView literal)
{
    for (size_t idx = 0; idx < literal.length(); ++idx) {
        builder.append(literal[idx]);
        if (literal[idx] == '{' || literal[idx] == '}')
            ++idx;
    }
}

[[gnu::noinline]] [[gnu::hot]] bool format_impl_head(StringBuilder& builder, StringView& fmt, StringView& specifier);
[[gnu::noinline]] [[gnu::hot]] bool format_impl_tail(StringBuilder& builder, StringView& fmt);

inline bool format(StringBuilder& builder, StringView fmt)
{
    return format_impl_tail(builder, fmt);
}
template<typename Parameter, typename... Parameters>
bool format(StringBuilder& builder, StringView fmt, const Parameter& parameter, const Parameters&... parameters)
{
    StringView specifier;
    if(!format_impl_head(builder, fmt, specifier))
        return false;

    Formatter<Parameter> formatter;
    if (!formatter.parse(specifier))
        return false;

    formatter.format(builder, parameter);

    return format(builder, fmt, parameters...);
}

}

namespace AK {

template<typename... Parameters>
String format(StringView fmt, const Parameters&... parameters)
{
    StringBuilder builder;
    if (!Detail::Format::format(builder, fmt, parameters...))
        ASSERT_NOT_REACHED();

    return builder.to_string();
}

template<>
struct Formatter<const char*> {
    [[gnu::noinline]] bool parse(StringView) { return true; }
    [[gnu::noinline]] void format(StringBuilder& builder, const char* value) { builder.append(value); }
};
template<size_t Size>
struct Formatter<char[Size]> {
    [[gnu::noinline]] bool parse(StringView) { return true; }
    [[gnu::noinline]] void format(StringBuilder& builder, const char* value) { builder.append(value); }
};
template<>
struct Formatter<StringView> {
    [[gnu::noinline]] bool parse(StringView) { return true; }
    [[gnu::noinline]] void format(StringBuilder& builder, StringView value) { builder.append(value); }
};
template<>
struct Formatter<String> {
    [[gnu::noinline]] bool parse(StringView) { return true; }
    [[gnu::noinline]] void format(StringBuilder& builder, const String& value) { builder.append(value); }
};
template<>
struct Formatter<u32> {
    [[gnu::noinline]] bool parse(StringView);
    [[gnu::noinline]] void format(StringBuilder& builder, u32 value);

    bool zero_pad { false };
    u32 field_width { 0 };
};

} // namespace AK
