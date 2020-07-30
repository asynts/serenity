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

TEST_CASE(keep_byte_order)
{
    const u32 expected = 0x01020304;
    u32 actual;

    InputMemoryStream stream { { &expected, sizeof(expected) } };
    stream >> actual;

    // EXPECT_EQ(expected, actual);
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

    EXPECT(actual.size() >= 256);

    for (int idx = 0; idx < 256; ++idx) {
        EXPECT_EQ(expected[idx], actual[idx]);
    }
}
*/

TEST_MAIN(MemoryStream)
