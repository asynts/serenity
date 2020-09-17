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

#include <AK/TestSuite.h>

#include <AK/Format.h>
#include <AK/StdLibExtras.h>

using namespace AK::Detail::Format;

TEST_CASE(tuple)
{
    const Tuple<int, float, int> tuple { 1, 2.11f, 3 };

    static_assert(IsSame<decltype(get<1>(tuple)), const float&>::value);

    EXPECT_EQ(get<0>(tuple), 1);
    EXPECT_EQ(get<1>(tuple), 2.11f);
    EXPECT_EQ(get<2>(tuple), 3);
}

struct A {
};

template<>
struct AK::Formatter<A> {
    bool parse(StringView fmtstr)
    {
        EXPECT_EQ(fmtstr, "x");

        b_parsed = true;
        return true;
    }

    bool b_parsed = false;
};

TEST_CASE(custom_formatter)
{
    Context<A> context;

    EXPECT((parse<0, decltype(context), A>(context, "a {x} b ")));
    EXPECT(context.formatters.value.b_parsed);
    EXPECT_EQ(context.literals[0], "a ");
    EXPECT_EQ(context.literals[1], " b ");
}

TEST_MAIN(Format)
