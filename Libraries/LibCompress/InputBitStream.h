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

#include <AK/Span.h>
#include <AK/Stream.h>

namespace Compress {

class InputBitStream final : public InputStream {
public:
    InputBitStream(InputStream& stream)
        : m_stream(stream)
    {
    }

    bool are_errors_recoverable() const override { return false; }

    size_t read(Bytes bytes) override
    {
        const auto nbytes_from_buffer = min(bytes.size(), m_buffered_data_length);

        ReadonlyBytes { m_buffered_data, nbytes_from_buffer }.copy_to(bytes);

        ReadonlyBytes { m_buffered_data + nbytes_from_buffer, m_buffered_data_length - nbytes_from_buffer }
            .move_to(Bytes { m_buffered_data, sizeof(m_buffered_data) });

        m_bit_offset = 0;
        m_buffered_data_length -= nbytes_from_buffer;

        return nbytes_from_buffer + m_stream.read(bytes.slice(nbytes_from_buffer));
    }

    bool read_or_error(Bytes bytes) override
    {
        if (read(bytes) < bytes.size()) {
            m_error = true;
            return false;
        }

        return true;
    }

    bool eof() const override { return m_buffered_data_length == 0 && m_stream.eof(); }

    bool discard_or_error(size_t count) override
    {
        const auto discarded_from_buffer = min(count, m_buffered_data_length);
        m_buffered_data_length = 0;

        return discard_or_error(count - discarded_from_buffer);
    }

    void read_bits(u32& value, size_t count)
    {
        ASSERT(count <= 32);

        const auto nbits_buffered = m_buffered_data_length * 8 - m_bit_offset;

        if (nbits_buffered >= count) {
            ReadonlyBytes { m_buffered_data, m_buffered_data_length }
                .copy_trimmed_to({ &value, sizeof(value) });

            value = value >> m_bit_offset;

            // FIXME: Don't move the buffer over but use m_bit_offset * 8 instead.

            // FIXME: Move the buffer over.

            m_bit_offset += count;
        }
    }

private:
    size_t m_bit_offset { 0 };
    u8 m_buffered_data[8];
    u8 m_buffered_data_length { 0 };
    InputStream& m_stream;
};

}
