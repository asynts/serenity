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

// This header is notorious for circular dependencies. We have to be very careful
// which headers we include because <AK/String.h>, <AK/StringBuilder.h> and
// <AK/LogStream.h> will cause issues.

#include <AK/Array.h>
#include <AK/Span.h>
#include <AK/StringView.h>

namespace AK {

template<typename T, typename = void>
struct Formatter;

} // namespace AK

namespace AK::FormatImplementation {

using TypeErasedAppender = void (*)(char);

struct TypeErasedParameter {
    const void* value;
    void (*format)(Context& context, const void* value, StringView flags);
};

struct Context {
    void append(char ch) { m_appender(ch); }
    void append(StringView value)
    {
        for (char ch : value)
            append(ch);
    }

    TypeErasedAppender m_appender;
    Span<const TypeErasedParameter> m_parameters;
    size_t m_next_index { 0 };
};

template<typename T>
void format_argument(Context& context, const void* value, StringView flags)
{
    Formatter<T> formatter;

    if (!formatter.parse(flags))
        ASSERT_NOT_REACHED();

    formatter.format(context, value);
}

void vformat(Context& context, StringView fmtstr);

template<typename... Parameters>
void format(TypeErasedAppender appender, StringView fmtstr, const Parameters&... parameters)
{
    Array<TypeErasedParameter, sizeof...(parameters)> type_erased_parameters { TypeErasedParameter { &parameters, format_argument<Parameters> }... };
    Context context { appender, type_erased_parameters };

    vformat(context, fmtstr);
}

struct StandardFormatter {
    bool parse(StringView flags);

    u8 base { 10 };
    u8 field_width { 0 };
    bool zero_pad { false };
};

} // namespace AK::FormatImplementation

namespace AK {

using FormatterContext = FormatImplementation::Context;

template<>
struct Formatter<StringView> : FormatImplementation::StandardFormatter {
    void format(FormatterContext& context, StringView value);
};
template<>
struct Formatter<String> : Formatter<StringView> {
};
template<size_t Size>
struct Formatter<char[Size]> : Formatter<StringView> {
};

template<typename T>
struct Formatter<T, typename EnableIf<IsIntegral<T>::value>::Type> : FormatImplementation::StandardFormatter {
    void format(FormatterContext& context, T value);
};

} // namespace AK
