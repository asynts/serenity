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

namespace AK {

// FIXME: Implement Buffered<T> for OutputStream and DuplexStream as well.

template<typename StreamType, size_t Size = 4096>
class Buffered final : public InputStream {
public:
    static_assert(IsBaseOf<InputStream, StreamType>::value);

    template<typename... Parameters>
    Buffered(Parameters&&... parameters)
        : m_stream(forward<Parameters>(parameters)...)
    {
    }

    bool has_recoverable_error() const override { return m_stream.has_recoverable_error(); }
    bool has_fatal_error() const override { return m_stream.has_fatal_error(); }
    bool has_any_error() const override { return m_stream.has_any_error(); }

    bool handle_recoverable_error() override { return m_stream.handle_recoverable_error(); }
    bool handle_fatal_error() override { return m_stream.handle_fatal_error(); }
    bool handle_any_error() override { return m_stream.handle_any_error(); }

    void set_recoverable_error() const override { return m_stream.set_recoverable_error(); }
    void set_fatal_error() const override { return m_stream.set_fatal_error(); }

    size_t read(Bytes bytes) override
    {
        const auto nread = buffer().trim(m_buffer_remaining).copy_trimmed_to(bytes);
        m_buffer_remaining -= nread;

        if (nread < bytes.size()) {
            m_buffer_remaining = m_stream.read(buffer());

            if (m_buffer_remaining == 0)
                return nread;

            return nread + read(bytes.slice(nread));
        }

        return nread;
    }

    virtual bool read_or_error(Bytes bytes) override
    {
        if (read(bytes) < bytes.size()) {
            set_fatal_error();
            return false;
        }

        return true;
    }

    virtual bool eof() const
    {
        if (m_buffer_remaining > 0)
            return false;

        m_buffer_remaining = m_stream.read(buffer());

        return m_buffer_remaining == 0;
    }

    virtual bool discard_or_error(size_t count) override
    {
        size_t ndiscarded = 0;
        while (ndiscarded < count) {
            u8 dummy[Size];

            if (!read_or_error({ dummy, min(Size, count - ndiscarded) }))
                return false;

            ndiscarded += min(Size, count - ndiscarded);
        }

        return true;
    }

private:
    Bytes buffer() const { return { m_buffer, Size }; }

    mutable StreamType m_stream;
    mutable u8 m_buffer[Size];
    mutable size_t m_buffer_remaining { 0 };
};

}

using AK::Buffered;
