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

#include <AK/Stream.h>

namespace Compress {

class OutputBitStream : public OutputStream {
public:
    explicit OutputBitStream(OutputStream& stream)
        : m_stream(stream)
    {
    }

    size_t write(ReadonlyBytes bytes) override
    {
        if (has_any_error())
            return 0;

        if (!align_to_byte_boundary_with_zero_fill())
            return 0;

        return m_stream.write(bytes);
    }

    bool write_or_error(ReadonlyBytes bytes) override
    {
        if (write(bytes) != bytes.size()) {
            set_fatal_error();
            return false;
        }

        return true;
    }

    bool write_bit(u16 value)
    {
        m_buffer = m_buffer >> 1 | value << 31;
        ++m_buffered;

        return flush_if_needed();
    }

    bool write_bits(u16 value, size_t count)
    {
        m_buffer = m_buffer >> count | value << (31 - count);
        m_buffered += count;

        return flush_if_needed();
    }

private:
    bool align_to_byte_boundary_with_zero_fill()
    {
        return write_bits(0, 8 - m_buffered % 8) && flush();
    }

    bool flush_if_needed()
    {
        if (m_buffered >= 16)
            return flush();

        return true;
    }

    bool flush()
    {
        const auto bytes_buffered = m_buffered / 8;

        u32 value = convert_between_host_and_big_endian(m_buffer << m_buffered % 8);
        m_stream.write_or_error({ &value, bytes_buffered });
        m_buffered -= 8 * bytes_buffered;

        return has_any_error();
    }

    u32 m_buffer = 0;
    size_t m_buffered = 0;

    OutputStream& m_stream;
};

}
