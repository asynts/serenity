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

#include <AK/FixedArray.h>
#include <AK/Span.h>
#include <AK/StdLibExtras.h>
#include <AK/Stream.h>

namespace AK {

// Extends InputStream with functionallity to read indivitual bits. Behaves just like
// the underlying stream, but consumes one byte from the stream when eight bits are
// read indivitually. Partially read bits are considered untouched and will still be
// considered by InputBitStream::read(Bytes).
class InputBitStream final : public InputStream {
public:
    InputBitStream(InputStream& stream)
        : m_stream(stream)
    {
        m_buffer.bytes().fill(0);
    }

    bool are_errors_recoverable() const override { return false; }

    size_t read(Bytes bytes) override
    {
        const auto nbytes_from_buffer = min(bytes.size(), m_buffer_size);

        m_buffer.bytes().slice(0, nbytes_from_buffer).copy_to(bytes);
        m_buffer.bytes().slice(nbytes_from_buffer, m_buffer_size - nbytes_from_buffer).move_to(m_buffer);

        m_bit_offset = 0;
        m_buffer_size -= nbytes_from_buffer;

        return nbytes_from_buffer + m_stream.read(m_buffer.bytes().slice(nbytes_from_buffer));
    }

    bool read_or_error(Bytes bytes) override
    {
        if (read(bytes) < bytes.size()) {
            m_error = true;
            return false;
        }

        return true;
    }

    bool eof() const override { return m_buffer_size == 0 && m_stream.eof(); }

    bool discard_or_error(size_t count) override
    {
        const auto discarded_from_buffer = min(count, m_buffer_size);
        m_buffer_size -= discarded_from_buffer;

        return discard_or_error(count - discarded_from_buffer);
    }

    u32 read_bits(size_t count)
    {
        ASSERT(count <= 32);

        ensure_bits_buffered(count);

        u64 value = *reinterpret_cast<u64*>(m_buffer.data());
        value = (value >> m_bit_offset % 8) & ((1ul << count) - 1);

        m_bit_offset += count;

        const auto bytes_consumed = m_bit_offset / 8;
        m_buffer.bytes().slice(bytes_consumed).move_to(m_buffer);
        m_bit_offset %= 8;

        return static_cast<u32>(value);
    }

private:
    void ensure_bits_buffered(size_t count)
    {
        if (m_buffer_size * 8 - m_bit_offset < count) {
            m_buffer_size += m_stream.read(m_buffer.bytes().slice(m_buffer_size));
        }

        ASSERT(m_buffer_size * 8 - m_bit_offset >= count);
    }

    FixedArray<u8> m_buffer { 8 };
    size_t m_buffer_size { 0 };
    size_t m_bit_offset { 0 };
    InputStream& m_stream;
};

}
