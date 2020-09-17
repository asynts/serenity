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
#include <AK/StringView.h>

namespace AK {

template<typename T>
class Formatter;

} // namespace AK

namespace AK::Detail {

template<typename T, typename... Types>
struct Tuple {
    T value;
    Tuple<Types...> remaining;
};
template<typename T>
struct Tuple<T> {
    T value;
};

template<size_t Index, typename T, typename... Types>
struct TupleElement {
    static_assert(Index < sizeof...(Types) + 1, "index out of range");

    using Type = typename TupleElement<Index - 1, Types...>::Type;

    static const Type& value(const Tuple<T, Types...>& tuple) { return TupleElement<Index - 1, Types...>::value(tuple.remaining); }
    static Type& value(Tuple<T, Types...>& tuple) { return TupleElement<Index - 1, Types...>::value(tuple.remaining); }
};
template<typename T, typename... Types>
struct TupleElement<0, T, Types...> {
    using Type = T;

    static const Type& value(const Tuple<T, Types...>& tuple) { return tuple.value; }
    static Type& value(Tuple<T, Types...>& tuple) { return tuple.value; }
};

template<size_t Index, typename... Types>
const typename TupleElement<Index, Types...>::Type& get(const Tuple<Types...>& tuple) { return TupleElement<Index, Types...>::value(tuple); }

template<size_t Index, typename... Types>
typename TupleElement<Index, Types...>::Type& get(Tuple<Types...>& tuple) { return TupleElement<Index, Types...>::value(tuple); }

template<typename... Types>
struct Context {
    Detail::Tuple<Formatter<Types>...> formatters;
    Array<StringView, sizeof...(Types) + 1> literals;
};

template<size_t Index, typename Context>
bool parse(Context& context, StringView fmtstr)
{
    // FIXME: Verify that there are no dangling format specifiers.

    context.literals[Index] = fmtstr;
    return true;
}

template<size_t Index, typename Context, typename Parameter, typename... Parameters>
bool parse(Context& context, StringView fmtstr)
{
    // FIXME: Allow escaping curly braces with double braces.
    // FIXME: Verify that the format specifier is correct.
    auto begin = fmtstr.find_first_of('{').value();
    auto end = fmtstr.find_first_of('}').value();

    context.literals[Index] = fmtstr.substring_view(0, begin);

    if (!Detail::get<Index>(context.formatters).parse(fmtstr.substring_view(begin + 1, end - (begin + 1))))
        return false;

    return parse<Index - 1, Context, Parameters...>(context, fmtstr.substring_view(end + 1));
}

} // namespace AK::Detail

namespace AK {

} // namespace AK
