/*
 * Copyright (c) 2021, Paul Scharnofske
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

#include <AK/Chrono.h>

TEST_CASE(basic_arithmetic)
{
    Chrono::Nanoseconds duration1 { 23 };
    Chrono::Nanoseconds duration2 { 42 };
    Chrono::Nanoseconds duration3 { -24 };
    Chrono::NanosecondsSinceEpoch instant1 { 100 };

    EXPECT_EQ(duration1 + duration2, 65ns);
    EXPECT_EQ(duration2 - duration1, 19ns);
    EXPECT_EQ(duration3 + duration1, -1ns);
    EXPECT_EQ((instant1 - duration1) - instant1, -23ns);
}

TEST_CASE(cast_durations)
{
    Chrono::Nanoseconds duration1 { 23 };
    Chrono::Milliseconds duration2 { 3 };

    EXPECT_EQ(duration1.in_milliseconds(), 0ms);
    EXPECT_EQ(duration2.in_nanoseconds(), 3'000'000ns);
}

TEST_CASE(weird_conversion_factors)
{
    constexpr Chrono::ClockRatio clock_ratio_1 { 1, 42 };
    constexpr Chrono::ClockRatio clock_ratio_2 { 2, 13 };

    Chrono::Duration<clock_ratio_1> duration1 { 123 };
    Chrono::Duration<clock_ratio_2> duration2 { -13 };

    EXPECT_EQ(duration1.cast<clock_ratio_2>().ticks(), 19);
    EXPECT_EQ(duration2.cast<clock_ratio_1>().ticks(), -84);
}

TEST_MAIN(Chrono)
