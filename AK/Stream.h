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

#include <AK/Concepts.h>
#include <AK/Forward.h>
#include <AK/Span.h>
#include <AK/StdLibExtras.h>

namespace AK::Detail {

class Stream {
public:
    virtual ~Stream()
    {
        ASSERT(!has_error());
    }

    bool has_error() const { return m_error; }

    bool handle_error() { return exchange(m_error, false); }

protected:
    mutable bool m_error { false };
};

}

namespace AK {

class InputStream : public virtual AK::Detail::Stream {
public:
    virtual size_t read(Bytes) = 0;
    virtual bool read_or_error(Bytes) = 0;
    virtual bool eof() const = 0;
    virtual bool discard_or_error(size_t count) = 0;
};

class OutputStream : public virtual AK::Detail::Stream {
public:
    virtual size_t write(ReadonlyBytes) = 0;
    virtual bool write_or_error(ReadonlyBytes) = 0;
};

class DuplexStream
    : public InputStream
    , public OutputStream {
};

#if defined(__cpp_concepts) && !defined(__COVERITY__)
template<Concepts::Integral Integral>
#else
template<typename Integral, typename EnableIf<IsIntegral<Integral>::value, int>::Type = 0>
#endif
InputStream& operator>>(InputStream& stream, Integral& value)
{
    stream.read_or_error({ &value, sizeof(value) });
    return stream;
}

#if defined(__cpp_concepts) && !defined(__COVERITY__)
template<Concepts::Integral Integral>
#else
template<typename Integral, typename EnableIf<IsIntegral<Integral>::value, int>::Type = 0>
#endif
OutputStream& operator<<(OutputStream& stream, Integral value)
{
    stream.write_or_error({ &value, sizeof(value) });
    return stream;
}

#ifndef KERNEL

#    if defined(__cpp_concepts) && !defined(__COVERITY__)
template<Concepts::FloatingPoint FloatingPoint>
#    else
template<typename FloatingPoint, typename EnableIf<IsFloatingPoint<FloatingPoint>::value, int>::Type = 0>
#    endif
InputStream& operator>>(InputStream& stream, FloatingPoint& value)
{
    stream.read_or_error({ &value, sizeof(value) });
    return stream;
}

#    if defined(__cpp_concepts) && !defined(__COVERITY__)
template<Concepts::FloatingPoint FloatingPoint>
#    else
template<typename FloatingPoint, typename EnableIf<IsFloatingPoint<FloatingPoint>::value, int>::Type = 0>
#    endif
OutputStream& operator<<(OutputStream& stream, FloatingPoint value)
{
    stream.write_or_error({ &value, sizeof(value) });
    return stream;
}

#endif

inline InputStream& operator>>(InputStream& stream, bool& value)
{
    stream.read_or_error({ &value, sizeof(value) });
    return stream;
}

inline OutputStream& operator<<(OutputStream& stream, bool value)
{
    stream.write_or_error({ &value, sizeof(value) });
    return stream;
}

inline InputStream& operator>>(InputStream& stream, Bytes bytes)
{
    stream.read_or_error(bytes);
    return stream;
}

inline OutputStream& operator<<(OutputStream& stream, ReadonlyBytes bytes)
{
    stream.write_or_error(bytes);
    return stream;
}

class InputMemoryStream final : public InputStream {
    friend InputMemoryStream& operator>>(InputMemoryStream& stream, String& string);

public:
    InputMemoryStream(ReadonlyBytes bytes)
        : m_bytes(bytes)
    {
    }

    bool eof() const override { return m_offset >= m_bytes.size(); }

    size_t read(Bytes bytes) override
    {
        const auto count = min(bytes.size(), remaining());
        __builtin_memcpy(bytes.data(), m_bytes.data() + m_offset, count);
        m_offset += count;
        return count;
    }

    bool read_or_error(Bytes bytes) override
    {
        if (remaining() < bytes.size()) {
            m_error = true;
            return false;
        }

        __builtin_memcpy(bytes.data(), m_bytes.data() + m_offset, bytes.size());
        m_offset += bytes.size();
        return true;
    }

    bool discard_or_error(size_t count) override
    {
        if (remaining() < count) {
            m_error = true;
            return false;
        }

        m_offset += count;
        return true;
    }

    void seek(size_t offset)
    {
        ASSERT(offset < m_bytes.size());
        m_offset = offset;
    }

    u8 peek_or_error() const
    {
        if (remaining() == 0) {
            m_error = true;
            return 0;
        }

        return m_bytes[m_offset];
    }

    // LEB128 is a variable-length encoding for integers
    bool read_LEB128_unsigned(size_t& result)
    {
        const auto backup = m_offset;

        result = 0;
        size_t num_bytes = 0;
        while (true) {
            // Note. The implementation in AK::BufferStream::read_LEB128_unsigned read one
            //       past the end, this is fixed here.
            if (eof()) {
                m_offset = backup;
                m_error = true;
                return false;
            }

            const u8 byte = m_bytes[m_offset];
            result = (result) | (static_cast<size_t>(byte & ~(1 << 7)) << (num_bytes * 7));
            ++m_offset;
            if (!(byte & (1 << 7)))
                break;
            ++num_bytes;
        }

        return true;
    }

    // LEB128 is a variable-length encoding for integers
    bool read_LEB128_signed(ssize_t& result)
    {
        const auto backup = m_offset;

        result = 0;
        size_t num_bytes = 0;
        u8 byte = 0;

        do {
            // Note. The implementation in AK::BufferStream::read_LEB128_unsigned read one
            //       past the end, this is fixed here.
            if (eof()) {
                m_offset = backup;
                m_error = true;
                return false;
            }

            byte = m_bytes[m_offset];
            result = (result) | (static_cast<size_t>(byte & ~(1 << 7)) << (num_bytes * 7));
            ++m_offset;
            ++num_bytes;
        } while (byte & (1 << 7));

        if (num_bytes * 7 < sizeof(size_t) * 4 && (byte & 0x40)) {
            // sign extend
            result |= ((size_t)(-1) << (num_bytes * 7));
        }

        return true;
    }

    ReadonlyBytes bytes() const { return m_bytes; }
    size_t offset() const { return m_offset; }
    size_t remaining() const { return m_bytes.size() - m_offset; }

private:
    ReadonlyBytes m_bytes;
    size_t m_offset { 0 };
};

class DuplexMemoryStream final : public DuplexStream {
public:
    bool eof() const override { return eof(0); }

    bool eof(size_t count) const
    {
        return m_write_offset - m_read_offset > count;
    }

    bool discard_or_error(size_t count) override
    {
        if (m_write_offset - m_read_offset < count) {
            m_error = true;
            return false;
        }

        m_read_offset += count;
        try_discard_chunks();
        return true;
    }

    size_t read(Bytes bytes) override
    {
        size_t nread = 0;
        while (bytes.size() - nread > 0 && !eof(nread)) {
            const auto chunk_index = (m_read_offset - m_base_offset) / chunk_size;
            const auto chunk_bytes = m_chunks[chunk_index].bytes().slice(m_read_offset % chunk_size).trim(m_write_offset - m_read_offset);
            nread += chunk_bytes.copy_to(bytes.slice(nread));
        }

        m_read_offset += nread;
        try_discard_chunks();
        return nread;
    }

    bool read_or_error(Bytes bytes) override
    {
        if (m_write_offset - m_read_offset < bytes.size()) {
            m_error = true;
            return false;
        }

        read(bytes);
        return true;
    }

    size_t write(ReadonlyBytes bytes) override
    {
        size_t nwritten = 0;
        while (bytes.size() - nwritten > 0) {
            if ((m_write_offset + nwritten) % chunk_size == 0)
                m_chunks.append(ByteBuffer::create_uninitialized(chunk_size));

            nwritten += bytes.copy_trimmed_to(m_chunks.last().bytes().slice(m_write_offset % chunk_size));
        }

        m_write_offset += nwritten;
        return nwritten;
    }

    bool write_or_error(ReadonlyBytes bytes) override
    {
        write(bytes);
        return true;
    }

private:
    static constexpr size_t chunk_size = 4096;
    static constexpr size_t history_size = 65536;

    void try_discard_chunks()
    {
        while (m_read_offset - m_base_offset >= chunk_size) {
            m_chunks.take_first();
            m_base_offset += chunk_size;
        }
    }

    Vector<ByteBuffer> m_chunks;
    size_t m_write_offset { 0 };
    size_t m_read_offset { 0 };
    size_t m_base_offset { 0 };
};
}

using AK::DuplexMemoryStream;
using AK::InputMemoryStream;
using AK::InputStream;
