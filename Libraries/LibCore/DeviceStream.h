/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
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
#include <LibCore/IODevice.h>

namespace Core {

class InputDeviceStream final : public InputStream {
public:
    InputDeviceStream(IODevice& device)
        : m_device(device)
    {
    }

    size_t read(Bytes bytes) override
    {
        int nread = m_device.read(bytes.data(), bytes.size());

        ASSERT(TypeBoundsChecker<size_t>::is_within_range(nread));
        return static_cast<size_t>(nread);
    }

    bool read_or_error(Bytes bytes) override
    {
        const auto nread = read(bytes);

        if (nread < bytes.size()) {
            const auto previous_size = m_device.m_buffered_data.size();
            m_device.m_buffered_data.grow_capacity(previous_size + nread);
            memmove(m_device.m_buffered_data.data() + nread, m_device.m_buffered_data.data(), previous_size);
            memcpy(m_device.m_buffered_data.data(), bytes.data(), nread);

            m_error = true;
            return false;
        }

        return true;
    }

    bool discard_or_error(size_t count)
    {
        const auto previously_in_buffer = m_device.m_buffered_data.size();
        if (count >= previously_in_buffer) {
            m_device.m_buffered_data.clear();

            auto buffer = ByteBuffer::create_uninitialized(count - previously_in_buffer);
            return read_or_error(buffer.span());
        }

        auto buffer = ByteBuffer::create_uninitialized(count);
        return read_or_error(buffer.span());
    }

    bool eof() const override { return m_device.eof(); }

private:
    IODevice& m_device;
};

}
