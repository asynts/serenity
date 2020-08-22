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

#include <AK/BitStream.h>
#include <AK/FixedArray.h>

void initialize_data(Bytes bytes)
{
    for (size_t idx = 0; idx < bytes.size(); ++idx) {
        bytes[idx] = static_cast<u8>(idx % 256);
    }
}

TEST_CASE(like_underlying_stream)
{
    FixedArray data { 1024 };
    initialize_data(data);

    InputMemoryStream stream0 { data };

    InputMemoryStream stream1_a { data };
    InputBitStream stream1_b { stream1_a };

    for (size_t idx = 0; idx < 1024 / 4; ++idx) {
        u32 value0 = 1, value1 = 2;

        stream0 >> value0;
        stream1_b >> value1;

        EXPECT_EQ(value0, value1);
        EXPECT_EQ(stream0.offset(), idx * 4);
    }

    EXPECT(stream0.eof() && stream1_b.eof());
}

TEST_CASE(partially_read_bits_are_considered)
{
    FixedArray data { 1 };
    data[0] = 0b11111110;

    u8 value = 0;

    InputMemoryStream stream0 { data };
    InputBitStream stream1 { stream0 };

    EXPECT_EQ(stream1.read_bits(1), 0);

    stream1 >> value;
    EXPECT_EQ(value, 0b11111110);
    EXPECT_EQ(stream0.offset(), 1);
}

TEST_CASE(read_accoss_byte_boundary)
{
    FixedArray data { 3 };
    data[0] = 0b01010101;
    data[1] = 0b11111111;
    data[2] = 0b11100011;

    InputMemoryStream stream0 { data };
    InputBitStream stream1 { stream0 };

    EXPECT_EQ(stream1.read_bits(3), 0b101);
    EXPECT_EQ(stream0.offset(), 0);

    EXPECT_EQ(stream1.read_bits(6), 0b101010);
    EXPECT_EQ(stream0.offset(), 1);

    EXPECT_EQ(stream1.read_bits(10), 0b0111111111);
    EXPECT_EQ(stream0.offset(), 2);

    EXPECT_EQ(stream1.read_bits(5), 0b11100);
    EXPECT_EQ(stream0.offset(), 3);

    EXPECT(stream0.eof() && stream1.eof());
}

TEST_MAIN(BitStream)
