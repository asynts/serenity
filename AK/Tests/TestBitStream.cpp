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
#include <AK/Stream.h>

bool are_bytes_equal(ReadonlyBytes lhs, ReadonlyBytes rhs)
{
    if (lhs.size() != rhs.size())
        return false;

    for (size_t idx = 0; idx < lhs.size(); ++idx) {
        if (lhs[idx] != rhs[idx])
            return false;
    }

    return true;
}

void initialize_input_data(FixedArray<u8>& array)
{
    for (size_t idx = 0; idx < array.size(); ++idx) {
        array[idx] = static_cast<u8>(idx % 256);
    }
}

TEST_CASE(like_underlying_stream)
{
    FixedArray<u8> input { 16 };
    initialize_input_data(input);

    InputMemoryStream stream0 { input };
    AK::InputBitStream stream1 { stream0 };

    FixedArray<u8> output1 { 16 };
    stream1.read_or_error(output1);

    ASSERT(are_bytes_equal(input, output1));
    ASSERT(stream0.eof() && stream1.eof());
    ASSERT(!stream0.has_error() && !stream1.has_error())
}

TEST_CASE(handle_errors)
{
    FixedArray<u8> input { 16 };
    initialize_input_data(input);

    InputMemoryStream stream0 { input };
    AK::InputBitStream stream1 { stream0 };

    FixedArray<u8> output1 { 16 };
    stream1.read_or_error(output1);

    ASSERT(stream1.handle_error());
}

TEST_CASE(read_bits_individually)
{
    FixedArray<u8> input { 4 };
    input[0] = 0b11110111;
    input[1] = 0b10101010;
    input[2] = 0b11000011;
    input[3] = 0b00000000;

    InputMemoryStream stream0 { input };
    AK::InputBitStream stream1 { stream0 };

    u32 value;

    value = stream1.read_bits(3);
    ASSERT(value == 0b111 && stream0.offset() == 0);

    value = stream1.read_bits(6);
    ASSERT(value == 0b011110 && stream0.offset() == 1);

    value = stream1.read_bits(2);
    ASSERT(value == 0b01 && stream0.offset() == 1);

    FixedArray<u8> output1 { 3 };
    stream1.read(output1);
    ASSERT(are_bytes_equal(input.bytes().slice(1), output1));
    ASSERT(stream0.eof() && stream1.eof());
    ASSERT(!stream0.has_error() && !stream1.has_error());
}

TEST_MAIN(BitStream)
