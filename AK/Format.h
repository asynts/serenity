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
class Formatter {
    bool parse(StringView);
    void format(StringBuilder&, const T&);
};

} // namespace AK

namespace AK::Detail::Format {

struct View {
    constexpr View()
        : data(nullptr)
        , length(0)
    {
    }
    constexpr View(const char* data, size_t length)
        : data(data)
        , length(length)
    {
    }
    constexpr View(const char* cstring)
        : data(cstring)
        , length(__builtin_strlen(cstring))
    {
    }
    constexpr View(StringView view)
        : data(view.characters_without_null_termination())
        , length(view.length())
    {
    }

    constexpr View substring(size_t start) const { return View { data + start, length - start }; }
    constexpr View substring(size_t start, size_t count) const { return View { data + start, count }; }

    constexpr char operator[](size_t index) const { return data[index]; }
    constexpr operator StringView() const { return StringView { data, length }; }

    constexpr bool find_first_of(size_t& index, char ch)
    {
        for (index = 0; index < length; ++index) {
            if (data[index] == ch)
                return true;
        }

        return false;
    }

    constexpr bool find_next_unescaped(size_t& index, char ch)
    {
        constexpr size_t unset = NumericLimits<size_t>::max();

        index = unset;
        for (size_t idx = 0; idx < length; ++idx) {
            if (data[idx] == ch) {
                if (index == unset)
                    index = idx;
                else
                    return true;
            } else if (index != unset) {
                return true;
            }
        }

        return false;
    }

    const char* data;
    size_t length;
};

template<typename... Types>
struct Context {
    View literal;
};
template<typename T, typename... Types>
struct Context<T, Types...> {
    View literal;
    Formatter<T> formatter;

    Context<Types...> next;
};

inline void write_escaped_literal(StringBuilder& builder, View literal)
{
    for (size_t idx = 0; idx < literal.length; ++idx) {
        builder.append(literal[idx]);
        if (literal[idx] == '{' || literal[idx] == '}')
            ++idx;
    }
}

template<size_t Index, typename Parameter, typename... Parameters>
bool parse(Context<Parameter, Parameters...>& context, View fmtstr)
{
    size_t start;
    if (!fmtstr.find_next_unescaped(start, '{'))
        return false;
    ++start;

    size_t length;
    if (!fmtstr.substring(start).find_first_of(length, '}'))
        return false;

    context.literal = fmtstr.substring(0, start - 1);

    context.formatter.parse(fmtstr.substring(start, length));

    return parse<Index + 1, Parameters...>(context.next, fmtstr.substring(start + length + 1));
}
template<size_t Index>
bool parse(Context<>& context, View fmtstr)
{
    size_t dummy;
    if (fmtstr.find_next_unescaped(dummy, '{') || fmtstr.find_next_unescaped(dummy, '}'))
        return false;

    context.literal = fmtstr;
    return true;
}

template<size_t Index, typename Parameter, typename... Parameters>
void format(StringBuilder& builder, Context<Parameter, Parameters...>& context, const Parameter& parameter, const Parameters&... parameters)
{
    write_escaped_literal(builder, context.literal);

    context.formatter.format(builder, parameter);

    format<Index + 1, Parameters...>(builder, context.next, parameters...);
}
template<size_t Index>
void format(StringBuilder& builder, Context<>& context)
{
    write_escaped_literal(builder, context.literal);
}

} // namespace AK::Detail::Format

namespace AK {

template<typename... Parameters>
String format(Detail::Format::View fmtstr, const Parameters&... parameters)
{
    Detail::Format::Context<Parameters...> context;

    if (!Detail::Format::parse<0, Parameters...>(context, fmtstr))
        ASSERT_NOT_REACHED();

    StringBuilder builder;
    Detail::Format::format<0, Parameters...>(builder, context, parameters...);

    return builder.to_string();
}

template<>
struct Formatter<StringView> {
    bool parse(Detail::Format::View) { return true; }
    void format(StringBuilder& builder, StringView value) { builder.append(value); }
};

template<>
struct Formatter<u32> {
    bool parse(Detail::Format::View);
    void format(StringBuilder&, u32);

    bool zero_pad { false };
    u32 field_width { 0 };
};

} // namespace AK
