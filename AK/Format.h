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
#include <AK/StringView.h>

namespace AK {

template<typename T>
class Formatter {
    bool parse(StringView);
    void format(StringBuilder&, const T&);
};

} // namespace AK

namespace AK::Detail::Format {

template<typename... Types>
struct Context {
    StringView literal;
};
template<typename T, typename... Types>
struct Context<T, Types...> {
    StringView literal;
    Formatter<T> formatter;

    Context<Types...> next;
};

Optional<size_t> find_next_unescaped(StringView input, char ch)
{
    Optional<size_t> index;
    for (size_t idx = 0; idx < input.length(); ++idx) {
        if (input[idx] == ch) {
            if (index.has_value())
                index.clear();
            else
                index = idx;
        } else if (index.has_value()) {
            return index;
        }
    }

    return index;
}

template<size_t Index, typename Parameter, typename... Parameters>
bool parse(Context<Parameter, Parameters...>& context, StringView fmtstr)
{
    auto begin = find_next_unescaped(fmtstr, '{');
    if (!begin.has_value())
        return false;

    auto end = begin.value() + fmtstr.substring_view(begin.value()).find_first_of('}');
    if (!end.has_value())
        return false;

    context.literal = fmtstr.substring_view(0, begin.value());

    context.formatter.parse(fmtstr.substring_view(begin.value() + 1, end.value - (begin.value() + 1)));

    return parse<Index + 1, Parameters...>(context.next, fmtstr.substring_view(end.value() + 1));
}
template<size_t Index>
bool parse(Context<>& context, StringView fmtstr)
{
    if (find_next_unescaped(fmtstr, '{') || find_next_unescaped(fmtstr, '}'))
        return false;

    context.literal = fmtstr;
    return true;
}

template<size_t Index, typename Parameter, typename... Parameters>
void format(StringBuilder& builder, Context<Parameter, Parameters...>& context, const Parameter& parameter, const Parameters&... parameters)
{
    for (size_t idx = 0; idx < context.literal.length(); ++idx) {
        builder.append(context.literal[idx]);
        if (context.literal[idx] == '{' || context.literal[idx] == '}')
            ++idx;
    }

    builder.append(context.literal);

    context.formatter.format(builder, parameter);

    format<Index + 1, Parameters...>(builder, context.next, parameters...);
}
template<size_t Index>
void format(StringBuilder& builder, Context<>& context)
{
    builder.append(context.literal);
}

} // namespace AK::Detail::Format

namespace AK {

template<typename... Parameters>
String format(StringView fmtstr, const Parameters&... parameters)
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
    bool parse(StringView) { return true; }
    void format(StringBuilder&, StringView);
};

} // namespace AK
