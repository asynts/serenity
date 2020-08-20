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

#include <LibCompress/Deflate.h>

bool compare(ReadonlyBytes lhs, ReadonlyBytes rhs)
{
    if (lhs.size() != rhs.size())
        return false;

    for (size_t idx = 0; idx < lhs.size(); ++idx) {
        if (lhs[idx] != rhs[idx])
            return false;
    }

    return true;
}

TEST_CASE(decompress_compressed_block)
{
    u8 compressed[] = {
        0x0B, 0xC9, 0xC8, 0x2C, 0x56, 0x00, 0xA2, 0x44, 0x85, 0xE2, 0xCC, 0xDC,
        0x82, 0x9C, 0x54, 0x85, 0x92, 0xD4, 0x8A, 0x12, 0x85, 0xB4, 0x4C, 0x20,
        0xCB, 0x4A, 0x13, 0x00
    };

    u8 uncompressed[] = "This is a simple text file :)";

    auto decompressed = Compress::DeflateStream::decompress_all({ compressed, sizeof(compressed) });
    EXPECT(compare(decompressed.span(), { uncompressed, sizeof(uncompressed) - 1 }));
}

TEST_CASE(decompress_uncompressed_block)
{
    u8 compressed[] = {
        0x0d, 0x00, 0xf2, 0xff, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57,
        0x6f, 0x72, 0x6c, 0x64, 0x21
    };

    u8 uncompressed[] = "Hello, World!";

    auto decompressed = Compress::DeflateStream::decompress_all({ compressed, sizeof(compressed) });
    EXPECT(compare(decompressed.span(), { uncompressed, sizeof(uncompressed) - 1 }));
}

TEST_CASE(decompress_multiple_blocks)
{
    u8 compressed[] = {
        0x2f, 0x00, 0xd0, 0xff, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
        0x74, 0x68, 0x65, 0x20, 0x66, 0x69, 0x72, 0x73, 0x74, 0x20, 0x62, 0x6c,
        0x6f, 0x63, 0x6b, 0x20, 0x77, 0x68, 0x69, 0x63, 0x68, 0x20, 0x69, 0x73,
        0x20, 0x75, 0x6e, 0x63, 0x6f, 0x6d, 0x70, 0x72, 0x65, 0x73, 0x73, 0x65,
        0x64, 0x2e, 0x0a, 0xc9, 0xc8, 0x2c, 0x56, 0x00, 0xa2, 0x92, 0x8c, 0x54,
        0x85, 0xe2, 0xd4, 0xe4, 0xfc, 0xbc, 0x14, 0x85, 0xa4, 0x9c, 0xfc, 0xe4,
        0x6c, 0x85, 0xf2, 0x8c, 0xcc, 0xe4, 0x0c, 0x90, 0x4c, 0x72, 0x7e, 0x6e,
        0x41, 0x51, 0x6a, 0x71, 0x71, 0x6a, 0x8a, 0x1e, 0x57, 0x08, 0x92, 0xea,
        0x92, 0x8c, 0xcc, 0x22, 0x0c, 0xc5, 0x89, 0x39, 0xc5, 0xf9, 0x28, 0x3a,
        0x00
    };

    u8 uncompressed[] = "This is the first block which is uncompressed.\n"
                        "This is the second block which is compressed.\n"
                        "This is the third block which is also compressed.\n";

    auto decompressed = Compress::DeflateStream::decompress_all({ compressed, sizeof(compressed) });
    EXPECT(compare(decompressed.span(), { uncompressed, sizeof(uncompressed) - 1 }));
}

TEST_MAIN(DeflateStream)
