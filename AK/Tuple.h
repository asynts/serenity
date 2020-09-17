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

#include <AK/Forward.h>

namespace AK {

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

}
