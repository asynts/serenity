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

#include <AK/MemoryStream.h>

TEST_CASE(input_memory_stream)
{
    const u8 buffer[] { 1, 2, 3, 4 };
    InputMemoryStream stream { { buffer, sizeof(buffer) } };

    u8 byte0, byte1, byte2, byte3;
    stream >> byte0 >> byte1 >> byte2 >> byte3;

    EXPECT(!stream.error());

    EXPECT_EQ(byte0, 1);
    EXPECT_EQ(byte1, 2);
    EXPECT_EQ(byte2, 3);
    EXPECT_EQ(byte3, 4);
}

TEST_CASE(output_memory_stream)
{
    u8 buffer[] { 0, 0 };
    OutputMemoryStream stream { { buffer, sizeof(buffer) } };

    stream << u8(1) << u8(2);

    EXPECT(!stream.error());

    EXPECT_EQ(buffer[0], 1);
    EXPECT_EQ(buffer[1], 2);
}

TEST_CASE(keeps_byte_order)
{
    const u32 expected = 0x01020304;

    InputMemoryStream stream { { &expected, sizeof(expected) } };

    u32 actual = 0;
    stream >> actual;

    EXPECT_EQ(expected, actual);
}

TEST_CASE(roundtrip)
{
    u8 expected[256];
    u8 actual[256];

    for (int idx = 0; idx < 256; ++idx) {
        expected[idx] = u8(idx);
        actual[idx] = 0;
    }

    InputMemoryStream stream0 { { expected, 256 } };
    OutputMemoryStream stream1 { { actual, 256 } };

    stream0 >> stream1;

    EXPECT(!stream0.error());
    EXPECT(!stream1.error());

    for (int idx = 0; idx < 256; ++idx) {
        EXPECT_EQ(expected[idx], actual[idx]);
    }
}

TEST_MAIN(MemoryStream)
