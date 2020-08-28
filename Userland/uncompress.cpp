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
#include <LibCore/FileStream.h>

static void decompress_gzip_file(StringView input_filename, StringView output_filename)
{
    auto input_stream_result = Core::InputFileStream::open(input_filename);
    auto output_stream_result = Core::OutputFileStream::open(output_filename);

    if (input_stream_result.is_error()) {
        out() << "error: can't open '" << input_filename << "', " << input_stream_result.error();
        exit(1);
    }

    if (output_stream_result.is_error()) {
        out() << "error: can't open '" << output_filename << "', " << output_stream_result.error();
        exit(1);
    }

    auto input_stream = input_stream_result.value();
    auto output_stream = output_stream_result.value();

    auto gzip_stream = Compress::GzipDecompressor { input_stream };

    u8 buffer[4096];

    while (!gzip_stream.eof()) {
        if (gzip_stream.has_error()) {
            out() << "error: '" << input_filename << "' is not a valid gzip file.";
            exit(1);
        }

        ASSERT(!output_stream.has_error());

        const auto nread = gzip_stream.read({ buffer, sizeof(buffer) });
        output_stream.write_or_error({ buffer, nread });
    }

    input_stream.close();
    output_stream.close();
}

int main(int argc, char** argv)
{
    Vector<const char*> filenames;
    bool use_gzip { false };
    bool do_discard { false };

    Core::ArgsParser args_parser;
    args_parser.add_option(use_gzip, "Input files are compressed using gzip.", "gzip", 0);
    args_parser.add_option(do_discard, "Discard input files after decompressing files.", "discard", 0);
    args_parser.add_positional_argument(filenames, "Files to decompress.", "FILE");
    args_parser.parse(argc, argv);

    ASSERT(use_gzip);

    for (StringView filename : filenames) {
        ASSERT(filename.ends_with(".gz"));

        String input_filename = filename;
        String output_filename = filename.substring_view(0, input_filename.length() - 3);

        decompress_gzip_file(input_filename, output_filename);

        if (do_discard) {
            if (unlink(input_filename.characters()))
                ASSERT_NOT_REACHED();
        }
    }
}
