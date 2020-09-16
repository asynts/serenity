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
#include <AK/Forward.h>
#include <AK/StringBuilder.h>
#include <AK/StringView.h>

// FIXME: Pass parameters by reference!

// FIXME: This was typed blindly, test all of this.

namespace AK {

template<typename...>
struct Tuple;

template<>
struct Tuple<> {
};

template<typename T, typename... Parameters>
struct Tuple<T, Parameters...> {
    T value;
    Tuple<Parameters...> remaining;
};

template<size_t Index, typename... Parameters>
class TupleElement;

template<typename T, typename... Parameters>
class TupleElement<0, T, Parameters...> {
    using Type = T;
};

template<size_t Index, typename T, typename... Parameters>
class TupleElement<Index, T, Parameters...> {
    using Type = typename TupleElement<sizeof...(Parameters), Parameters...>::Type;
};

template<size_t Index, typename T, typename... Parameters>
typename TupleElement<Index, T, Parameters...>::Type& get(Tuple<T, Parameters...>& tuple)
{
    if constexpr (Index == 0)
        return tuple.value;
    else
        return get<Index - 1, Parameters...>(tuple.remaining);
}

template<size_t Index, typename T, typename... Parameters>
const typename TupleElement<Index, T, Parameters...>::Type& get(const Tuple<T, Parameters...>& tuple)
{
    if constexpr (Index == 0)
        return tuple.value;
    else
        return get<Index - 1, Parameters...>(tuple.remaining);
}

template<typename T>
struct Formatter;

template<typename... Parameters>
struct Context {
    Array<StringView, 1 + sizeof...(Parameters)> literals;
    Tuple<Formatter<Parameters>...> formatters;
};

template<size_t Index, typename Context>
bool parse(Context& context, StringView fmtstr)
{
    bool saw_lparan = false, saw_rparen = false;
    for (auto ch : fmtstr) {
        if (ch == '{') {
            if (saw_rparen)
                return false;

            if (saw_lparan) {
                saw_lparan = false;
                continue;
            }

            saw_lparan = true;
            continue;
        }

        if (ch == '}') {
            if (saw_lparan)
                return false;

            if (saw_rparen) {
                saw_rparen = false;
                continue;
            }

            saw_rparen = true;
            continue;
        }

        if (saw_lparan || saw_rparen)
            return false;
    }

    if (fmtstr.contains('{') || fmtstr.contains('}'))
        return false;

    context.literals[Index] = fmtstr;
    return true;
}

template<size_t Index, typename Context, typename Parameter, typename... Parameters>
bool parse(Context& context, StringView fmtstr)
{
    auto begin = fmtstr.find_first_of('{').value();
    auto end = fmtstr.find_first_of('}').value();

    context.literals[Index] = fmtstr.substring_view(0, begin);

    ++begin;

    get<Index>(context.formatters).parse(fmtstr.substr(begin, end - begin));

    return parse<Index + 1, Context, Parameters...>(context, fmtstr.substring_view(end + 1));
}

template<size_t Index, typename Context>
void format_impl(Context& context, StringBuilder& builder)
{
    builder.append(context.literals[Index]);
}

template<size_t Index, typename Context, typename Parameter, typename... Parameters>
void format_impl(Context& context, StringBuilder& builder, Parameter parameter, Parameters... parameters)
{
    builder.append(context.literals[Index]);

    get<Index>(context.formatters).format(builder, parameter);

    format_impl<Index + 1, Context, Parameters...>(context, builder, parameters...);
}

template<typename... Parameters>
String format(StringView fmtstr, Parameters... parameters)
{
    Context<Parameters...> context;
    if (!parse<0, Context<Parameters...>, Parameters...>(context, fmtstr))
        ASSERT_NOT_REACHED();

    StringBuilder builder;

    format_impl<0, Context<Parameters...>, Parameters...>(context, builder, parameters...);

    return builder.to_string();
}

}
