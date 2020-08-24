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

#include <AK/CircularQueue.h>
#include <AK/Stream.h>

namespace Compress {

// FIXME: There are a lot of raw loops here, that's not necessary an issue but it
//        has to be verified that the optimizer is able to insert memcpy instead.
template<size_t Capacity>
class CircularDuplexStream : public AK::DuplexStream {
public:
    size_t write(ReadonlyBytes bytes) override
    {
        const auto nwritten = min(bytes.size(), Capacity - m_queue.size());

        for (size_t idx = 0; idx < nwritten; ++idx)
            m_queue.enqueue(bytes[idx]);

        m_total_written += nwritten;
        return nwritten;
    }

    bool write_or_error(ReadonlyBytes bytes) override
    {
        if (bytes.size() < Capacity - m_queue.size()) {
            m_error = true;
            return false;
        }

        write(bytes);
        return true;
    }

    size_t read(Bytes bytes) override
    {
        const auto nread = min(bytes.size(), m_queue.size());

        for (size_t idx = 0; idx < nread; ++idx)
            bytes[idx] = m_queue.dequeue();

        return nread;
    }

    size_t read(Bytes bytes, size_t seekback)
    {
        ASSERT(seekback <= 64 * 1024);

        if (seekback > m_total_written) {
            m_error = true;
            return 0;
        }

        const auto nread = min(bytes.size(), seekback);

        for (size_t idx = 0; idx < nread; ++idx) {
            const auto index = (m_total_written - seekback + idx) % Capacity;
            bytes[idx] = m_queue.m_storage[index];
        }

        return nread;
    }

    bool read_or_error(Bytes bytes) override
    {
        if (m_queue.size() < bytes.size()) {
            m_error = true;
            return false;
        }

        read(bytes);
        return true;
    }

    bool discard_or_error(size_t count) override
    {
        if (m_queue.size() < count) {
            m_error = true;
            return false;
        }

        for (size_t idx = 0; idx < count; ++idx)
            m_queue.dequeue();

        return true;
    }

    bool eof() const override { return m_queue.size() == 0; }

private:
    CircularQueue<u8, Capacity> m_queue;
    size_t m_total_written { 0 };
};

}
