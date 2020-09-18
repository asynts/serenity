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

inline Optional<size_t> find_next_unescaped(StringView input, char ch)
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

inline void write_escaped_literal(StringBuilder& builder, StringView literal)
{
    for (size_t idx = 0; idx < literal.length(); ++idx) {
        builder.append(literal[idx]);
        if (literal[idx] == '{' || literal[idx] == '}')
            ++idx;
    }
}

template<size_t Index, typename Parameter, typename... Parameters>
bool parse(Context<Parameter, Parameters...>& context, StringView fmtstr)
{
    auto start = find_next_unescaped(fmtstr, '{');
    if (!start.has_value())
        return false;
    ++start.value();

    auto length = fmtstr.substring_view(start.value()).find_first_of('}');
    if (!length.has_value())
        return false;

    context.literal = fmtstr.substring_view(0, start.value() - 1);

    context.formatter.parse(fmtstr.substring_view(start.value(), length.value()));

    return parse<Index + 1, Parameters...>(context.next, fmtstr.substring_view(start.value() + length.value() + 1));
}
template<size_t Index>
bool parse(Context<>& context, StringView fmtstr)
{
    if (find_next_unescaped(fmtstr, '{').has_value() || find_next_unescaped(fmtstr, '}').has_value())
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
    void format(StringBuilder& builder, StringView value) { builder.append(value); }
};

} // namespace AK
