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

InputStream& operator>>(InputStream& stream, u8& value)
{
    stream.read_or_error({ &value, sizeof(value) });
    return stream;
}

TEST_CASE(input_memory_stream)
{
#if __BYTE_ORDER__ == __LITTLE_ENDIAN__
    const u32 value = 0x04030201;
#else
    const u32 value = 0x01020304;
#endif

    InputMemoryStream stream { { &value, sizeof(value) } };

    u8 byte0, byte1, byte2, byte3;
    stream >> byte0 >> byte1 >> byte2 >> byte3;

    EXPECT(!stream.error());

    EXPECT_EQ(byte0, 1);
    EXPECT_EQ(byte1, 2);
    EXPECT_EQ(byte2, 3);
    EXPECT_EQ(byte3, 4);
}

/*
TEST_CASE(roundtrip)
{
    u8 expected[256];

    for (int idx = 0; idx < 256; ++idx) {
        expected[idx] = static_cast<u8>(idx);
    }

    auto actual = ByteBuffer::create_zeroed(128);

    InputMemoryStream stream0 { { expected, 256 } };
    OutputMemoryStream stream1 { actual };

    stream0 >> stream1;

    EXPECT(!stream0.error());
    EXPECT(!stream1.error());

    EXPECT(actual.size() >= 256);

    for (int idx = 0; idx < 256; ++idx) {
        EXPECT_EQ(expected[idx], actual[idx]);
    }
}
*/

TEST_MAIN(MemoryStream)
