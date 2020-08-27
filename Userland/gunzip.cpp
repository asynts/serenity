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
#include <LibCore/ArgsParser.h>
#include <LibCore/File.h>

// FIXME: Maybe add some buffering, or does Core::File do this already? Also, this should
//        be moved into LibCore.
class InputFileStream final : public InputStream {
public:
    InputFileStream(NonnullRefPtr<Core::File> file)
        : m_file(file)
    {
    }

    size_t read(Bytes bytes) override
    {
        dbg() << "read({ " << bytes.data() << ", " << bytes.size() << "})";

        size_t nread = 0;
        while (nread < bytes.size()) {
            if (m_file->eof())
                return nread;

            const auto buffer = m_file->read(nread - bytes.size());
            nread += buffer.bytes().copy_to(bytes.slice(nread));
        }

        return nread;
    }

    bool read_or_error(Bytes bytes) override
    {
        if (read(bytes) < bytes.size()) {
            m_error = true;
            return false;
        }

        return true;
    }

    bool discard_or_error(size_t count) override
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

    bool eof() const override { return m_file->eof(); }

private:
    InputFileStream() = default;

    NonnullRefPtr<Core::File> m_file;
};

// FIXME: Maybe add some buffering, or does Core::File do this already? Also, this should
//        be moved into LibCore.
class OutputFileStream : public OutputStream {
public:
    OutputFileStream(NonnullRefPtr<Core::File> file)
        : m_file(file)
    {
    }

    size_t write(ReadonlyBytes bytes) override
    {
        dbg() << "write({ " << bytes.data() << ", " << bytes.size() << "})";

        if (!m_file->write(bytes.data(), bytes.size())) {
            m_error = true;
            return 0;
        }

        return bytes.size();
    }

    bool write_or_error(ReadonlyBytes bytes) override
    {
        if (write(bytes) < bytes.size()) {
            m_error = true;
            return false;
        }

        return true;
    }

private:
    NonnullRefPtr<Core::File> m_file;
};

StringView output_filename_for(StringView filename)
{
    return filename.substring_view(0, filename.length() - 3);
}

void decompress_file(InputFileStream input_stream, OutputFileStream output_stream)
{
    auto gzip_stream = Compress::GzipDecompressor { input_stream };

    u8 buffer[4096];
    while (!gzip_stream.eof()) {
        ASSERT(!input_stream.has_error());
        ASSERT(!output_stream.has_error());
        ASSERT(!gzip_stream.has_error());

        const auto nread = gzip_stream.read({ buffer, sizeof(buffer) });
        output_stream.write({ buffer, nread });
    }
}

int main(int argc, char** argv)
{
    Vector<const char*> filenames;

    Core::ArgsParser args_parser;
    args_parser.add_positional_argument(filenames, "Path to gzip compressed file.", "FILE");

    args_parser.parse(argc, argv);

    for (StringView input_filename : filenames) {
        ASSERT(input_filename.ends_with(".gz"));
        const auto output_filename = input_filename.substring_view(0, input_filename.length() - 3);

        auto input_file_result = Core::File::open(input_filename, Core::IODevice::OpenMode::ReadOnly);
        auto output_file_result = Core::File::open(output_filename, Core::IODevice::OpenMode::WriteOnly);

        decompress_file(input_file_result.value(), output_file_result.value());
    }
}
