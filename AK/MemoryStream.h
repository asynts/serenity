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

#include <AK/ByteBuffer.h>
#include <AK/MemMem.h>
#include <AK/Stream.h>
#include <AK/Vector.h>

namespace AK {

class InputMemoryStream final : public InputStream {
public:
    InputMemoryStream(ReadonlyBytes bytes)
        : m_bytes(bytes)
    {
    }

    bool unreliable_eof() const override { return eof(); }
    bool eof() const { return m_offset >= m_bytes.size(); }

    size_t read(Bytes bytes) override
    {
        if (has_any_error())
            return 0;

        const auto count = min(bytes.size(), remaining());
        __builtin_memcpy(bytes.data(), m_bytes.data() + m_offset, count);
        m_offset += count;
        return count;
    }

    bool read_or_error(Bytes bytes) override
    {
        if (remaining() < bytes.size()) {
            set_recoverable_error();
            return false;
        }

        __builtin_memcpy(bytes.data(), m_bytes.data() + m_offset, bytes.size());
        m_offset += bytes.size();
        return true;
    }

    bool discard_or_error(size_t count) override
    {
        if (remaining() < count) {
            set_recoverable_error();
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
            set_recoverable_error();
            return 0;
        }

        return m_bytes[m_offset];
    }

    bool read_LEB128_unsigned(size_t& result)
    {
        const auto backup = m_offset;

        result = 0;
        size_t num_bytes = 0;
        while (true) {
            if (eof()) {
                m_offset = backup;
                set_recoverable_error();
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

    bool read_LEB128_signed(ssize_t& result)
    {
        const auto backup = m_offset;

        result = 0;
        size_t num_bytes = 0;
        u8 byte = 0;

        do {
            if (eof()) {
                m_offset = backup;
                set_recoverable_error();
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
    static constexpr size_t chunk_size = 4096;

    DuplexMemoryStream() { }

    explicit DuplexMemoryStream(Bytes buffer, bool allow_growth = false)
        : m_base_offset(buffer.size())
        , m_first_buffer(buffer)
        , m_allow_growth(allow_growth)
    {
    }

    bool unreliable_eof() const override { return eof(); }
    bool eof() const { return size() == 0; }

    bool discard_or_error(size_t count) override
    {
        if (size() < count) {
            set_recoverable_error();
            return false;
        }

        m_read_offset += count;
        try_discard_chunks();
        return true;
    }

    Optional<size_t> offset_of(ReadonlyBytes value) const
    {
        // FIXME: This implementation (and the previous implementation silently) does not consider chunk
        //        boundaries. That seems tough to implement so let's do that when it comes up.
        //
        //        We are only dealing with the cases where an inline buffer was provided and no additional
        //        chunks were allocated or where no inline buffer was provided and at most one chunk was allocated.
        if (m_first_buffer.size() > 0)
            ASSERT(m_chunks.size() == 0);
        else
            ASSERT(m_chunks.size() <= 1);

        if (m_first_buffer.size() > 0) {
            const auto position = AK::memmem(m_first_buffer.data(), m_first_buffer.size(), value.data(), value.size());

            if (!position)
                return {};

            return static_cast<size_t>(reinterpret_cast<const u8*>(position) - m_first_buffer.data());
        } else {
            const auto chunk = m_chunks.first();

            const auto position = AK::memmem(chunk.data(), chunk.size(), value.data(), value.size());

            if (!position)
                return {};

            return static_cast<size_t>(reinterpret_cast<const u8*>(position) - chunk.data());
        }
    }

    size_t read_without_consuming(Bytes bytes) const
    {
        if (has_any_error())
            return 0;

        size_t nread = 0;
        while (bytes.size() - nread > 0 && size() - nread > 0) {
            if (m_read_offset + nread < m_first_buffer.size()) {
                nread += m_first_buffer.copy_trimmed_to(bytes.slice(nread));
            } else {
                const auto offset_into_chunks = m_read_offset + nread - m_base_offset;
                const auto chunk_index = offset_into_chunks / chunk_size;
                const auto chunk_bytes = m_chunks[chunk_index].bytes().slice(offset_into_chunks % chunk_size).trim(size() - nread);
                nread += chunk_bytes.copy_trimmed_to(bytes.slice(nread));
            }
        }

        return nread;
    }

    size_t read(Bytes bytes) override
    {
        if (has_any_error())
            return 0;

        const auto nread = read_without_consuming(bytes);

        m_read_offset += nread;
        try_discard_chunks();

        return nread;
    }

    bool read_or_error(Bytes bytes) override
    {
        if (size() < bytes.size()) {
            set_recoverable_error();
            return false;
        }

        read(bytes);
        return true;
    }

    size_t write(ReadonlyBytes bytes) override
    {
        auto nwritten = bytes.copy_trimmed_to(m_first_buffer.slice(min(m_write_offset, m_first_buffer.size())));

        while (bytes.size() - nwritten > 0) {
            if (!m_allow_growth)
                ASSERT_NOT_REACHED();

            const auto offset_into_chunks = m_write_offset + nwritten - m_first_buffer.size();

            if (offset_into_chunks % chunk_size == 0)
                m_chunks.append(ByteBuffer::create_uninitialized(chunk_size));

            nwritten += bytes.copy_trimmed_to(m_chunks.last().bytes().slice(offset_into_chunks % chunk_size));
        }

        m_write_offset += nwritten;
        return nwritten;
    }

    bool write_or_error(ReadonlyBytes bytes) override
    {
        if (!m_allow_growth && m_first_buffer.size() - m_write_offset < bytes.size()) {
            set_recoverable_error();
            return false;
        }

        write(bytes);
        return true;
    }

    ByteBuffer copy_into_contiguous_buffer() const
    {
        auto buffer = ByteBuffer::create_uninitialized(size());
        read_without_consuming(buffer);

        return buffer;
    }

    size_t size() const { return m_write_offset - m_read_offset; }

private:
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
    Bytes m_first_buffer;
    bool m_allow_growth { true };
};

class OutputMemoryStream final : public OutputStream {
public:
    OutputMemoryStream() { }

    explicit OutputMemoryStream(Bytes buffer, bool allow_growth = false)
        : m_stream(buffer, allow_growth)
    {
    }

    size_t write(ReadonlyBytes bytes) override { return m_stream.write(bytes); }
    bool write_or_error(ReadonlyBytes bytes) override { return m_stream.write_or_error(bytes); }

    ByteBuffer copy_into_contiguous_buffer() const { return m_stream.copy_into_contiguous_buffer(); }
    Optional<size_t> offset_of(ReadonlyBytes value) const { return m_stream.offset_of(value); }

    size_t size() const { return m_stream.size(); }

    bool has_recoverable_error() const override { return m_stream.has_recoverable_error(); }
    bool has_fatal_error() const override { return m_stream.has_fatal_error(); }
    bool has_any_error() const override { return m_stream.has_any_error(); }
    bool handle_recoverable_error() override { return m_stream.handle_recoverable_error(); }
    bool handle_fatal_error() override { return m_stream.handle_fatal_error(); }
    bool handle_any_error() override { return m_stream.handle_any_error(); }
    void set_recoverable_error() const override { return m_stream.set_recoverable_error(); }
    void set_fatal_error() const override { return m_stream.set_fatal_error(); }

private:
    DuplexMemoryStream m_stream;
};

}

using AK::DuplexMemoryStream;
using AK::InputMemoryStream;
using AK::InputStream;
using AK::OutputMemoryStream;
