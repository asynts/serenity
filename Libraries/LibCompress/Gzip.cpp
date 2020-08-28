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

#include <LibCompress/Gzip.h>

#include <AK/String.h>

namespace Compress {

bool GzipDecompressor::BlockHeader::valid_magic_number() const
{
    return identification_1 == 0x1f && identification_2 == 0x8b;
}

bool GzipDecompressor::BlockHeader::supported_by_implementation() const
{
    if (compression_method != 0x08) {
        // RFC 1952 does not define any compression methods other than deflate.
        return false;
    }

    if (flags > Flags::MAX) {
        // RFC 1952 does not define any more flags.
        return false;
    }

    if (flags & Flags::FHCRC) {
        TODO();
    }

    return true;
}

GzipDecompressor::GzipDecompressor(InputStream& stream)
    : m_input_stream(stream)
{
}

GzipDecompressor::~GzipDecompressor()
{
    if (m_state == State::ReadingDeflateBlock) {
        m_block_stream.~DeflateDecompressor();
        m_block_header.~BlockHeader();
    }
}

// FIXME: Again, there are surely a ton of bugs because the code doesn't check for read errors.
size_t GzipDecompressor::read(Bytes bytes)
{
    if (m_state == State::ReadingDeflateBlock) {
        size_t nread = m_block_stream.read(bytes);

        if (nread < bytes.size()) {
            m_block_stream.~DeflateDecompressor();
            m_state = State::Idle;

            LittleEndian<u32> crc32, input_size;
            m_input_stream >> crc32 >> input_size;

            // FIXME: Validate crc32 and input_size.

            return nread + read(bytes.slice(nread));
        }

        return nread;
    }

    if (m_state == State::Idle) {
        if (m_input_stream.eof())
            return 0;

        new (&m_block_header) BlockHeader();
        m_input_stream >> Bytes { &m_block_header, sizeof(m_block_header) };

        if (!m_block_header.valid_magic_number() || !m_block_header.supported_by_implementation()) {
            m_error = true;
            return 0;
        }

        // We are simply discarding the extra field because we don't understand it.
        if (m_block_header.flags & Flags::FEXTRA) {
            u16 subfield_id, length;
            m_input_stream >> subfield_id >> length;
            m_input_stream.discard_or_error(length);
        }

        // The original filename is obsolete, we just discard it.
        if (m_block_header.flags & Flags::FNAME) {
            String original_filename;
            m_input_stream >> original_filename;
        }

        if (m_block_header.flags & Flags::FCOMMENT) {
            String comment;
            m_input_stream >> comment;
        }

        m_state = State::ReadingDeflateBlock;
        new (&m_block_stream) DeflateDecompressor(m_input_stream);

        return read(bytes);
    }

    ASSERT_NOT_REACHED();
}

bool GzipDecompressor::read_or_error(Bytes bytes)
{
    if (read(bytes) < bytes.size()) {
        m_error = true;
        return false;
    }

    return true;
}

bool GzipDecompressor::discard_or_error(size_t count)
{
    u8 buffer[4096];

    size_t ndiscarded = 0;
    while (ndiscarded < count) {
        if (eof()) {
            m_error = true;
            return false;
        }

        ndiscarded += read({ buffer, min<size_t>(count - ndiscarded, sizeof(buffer)) });
    }

    return true;
}

ByteBuffer GzipDecompressor::decompress_all(ReadonlyBytes bytes)
{
    InputMemoryStream memory_stream { bytes };
    GzipDecompressor gzip_stream { memory_stream };

    auto buffer = ByteBuffer::create_uninitialized(4096);

    size_t nread = 0;
    while (!gzip_stream.eof()) {
        nread += gzip_stream.read(buffer.bytes().slice(nread));

        if (buffer.size() - nread < 4096)
            buffer.grow(buffer.size() + 4096);
    }

    buffer.trim(nread);
    return buffer;
}

bool GzipDecompressor::eof() const
{
    if (m_state == State::Idle)
        return m_input_stream.eof();

    if (m_state == State::ReadingDeflateBlock) {
        // FIXME: There is an ugly edge case where we read the whole deflate block
        //        but haven't read CRC32 and ISIZE.
        return m_block_stream.eof() && m_input_stream.eof();
    }

    ASSERT_NOT_REACHED();
}

}
