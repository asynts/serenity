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
#include <AK/GenericLexer.h>
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

struct FormatSpecifier {
    StringView flags;
    size_t index { 0 };
};

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
inline void write_escaped_literal(StringBuilder& builder, StringView literal)
{
    for (size_t idx = 0; idx < literal.length(); ++idx) {
        builder.append(literal[idx]);
        if (literal[idx] == '{' || literal[idx] == '}')
            ++idx;
    }
}
inline size_t parse_number(StringView input)
{
    // FIXME: We really don't want to do a heap allocation here. There should be
    //        some shared integer parsing code that is used in strtoll and similar
    //        methods.
    String null_terminated { input };
    char* endptr;
    return strtoull(null_terminated.characters(), &endptr, 10);
}
inline bool parse_format_specifier(StringView input, FormatSpecifier& specifier)
{
    specifier.index = NumericLimits<size_t>::max();

    GenericLexer lexer { input };

    auto index = lexer.consume_while([](char ch) { return StringView { "0123456789" }.contains(ch); });

    if (index.length() > 0)
        specifier.index = parse_number(index);

    if (!lexer.consume_specific(':'))
        return lexer.is_eof();

    specifier.flags = lexer.consume_all();
    return true;
}

[[gnu::hot, gnu::noinline]] inline bool format(StringBuilder& builder, StringView fmtstr, AK::Span<TypeErasedArgument> arguments, AK::Span<TypeErasedFormatter> formatters, size_t argument_index = 0)
{
    if (arguments.size() != formatters.size())
        ASSERT_NOT_REACHED();

    size_t opening;
    if (!find_next_unescaped(opening, fmtstr, '{')) {
        size_t dummy;
        if (find_next_unescaped(dummy, fmtstr, '}'))
            return false;

        write_escaped_literal(builder, fmtstr);
        return true;
    }

    write_escaped_literal(builder, fmtstr.substring_view(0, opening));

    size_t closing;
    if (!find_next_unescaped(closing, fmtstr.substring_view(opening), '}'))
        return false;
    closing += opening;

    FormatSpecifier specifier;
    if (!parse_format_specifier(fmtstr.substring_view(opening + 1, closing - (opening + 1)), specifier))
        return false;

    if (specifier.index == NumericLimits<size_t>::max())
        specifier.index = argument_index++;

    if (specifier.index >= arguments.size())
        return false;

    if (!formatters[specifier.index](builder, arguments[specifier.index], specifier.flags))
        return false;

    return format(builder, fmtstr.substring_view(closing + 1), arguments, formatters, argument_index);
}

} // namespace AK::Detail::Format

namespace AK {

template<typename... Parameters>
inline String format(StringView fmtstr, const Parameters&... parameters)
{
    Array<Detail::Format::TypeErasedFormatter, sizeof...(Parameters)> formatters { Detail::Format::format_value<Parameters>... };
    Array<Detail::Format::TypeErasedArgument, sizeof...(Parameters)> arguments { &parameters... };

    StringBuilder builder;
    if (!Detail::Format::format(builder, fmtstr, arguments, formatters))
        ASSERT_NOT_REACHED();

    return builder.to_string();
}

template<size_t Size>
struct Formatter<char[Size]> {
    bool parse(StringView) { return true; }
    void format(StringBuilder& builder, const char* value) { builder.append(value); }
};

}
