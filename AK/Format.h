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

#include <AK/Array.h>
#include <AK/String.h>
#include <AK/StringBuilder.h>

namespace AK {

template<typename T>
struct Formatter;

} // namespace AK

namespace AK::Detail::Format {

using TypeErasedFormatter = bool (*)(StringBuilder& builder, const void* value, StringView flags);
using TypeErasedArgument = const void*;

template<typename T>
bool format_value(StringBuilder& builder, const void* value, StringView flags)
{
    Formatter<T> formatter;

    if (!formatter.parse(flags))
        return false;

    formatter.format(builder, *static_cast<const T*>(value));
    return true;
}

void format(StringBuilder&, StringView fmtstr, AK::Span<TypeErasedArgument>, AK::Span<TypeErasedFormatter>, size_t argument_index = 0);

} // namespace AK::Detail::Format

namespace AK {

template<typename... Parameters>
inline String format(StringView fmtstr, const Parameters&... parameters)
{
    Array<Detail::Format::TypeErasedFormatter, sizeof...(Parameters)> formatters { Detail::Format::format_value<Parameters>... };
    Array<Detail::Format::TypeErasedArgument, sizeof...(Parameters)> arguments { &parameters... };

    StringBuilder builder;
    Detail::Format::format(builder, fmtstr, arguments, formatters);

    return builder.to_string();
}

template<size_t Size>
struct Formatter<char[Size]> {
    bool parse(StringView) { return true; }
    void format(StringBuilder& builder, const char* value) { builder.append(value); }
};

template<>
struct Formatter<u32> {
    bool parse(StringView);
    void format(StringBuilder&, u32);

    bool zero_pad { false };
    size_t field_width { 0 };
};

}
