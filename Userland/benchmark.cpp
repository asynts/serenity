#include <AK/Array.h>
#include <AK/MappedFile.h>
#include <AK/MemoryStream.h>
#include <LibCompress/Gzip.h>
#include <LibCore/ArgsParser.h>
#include <LibCore/Gzip.h>

#include <sys/time.h>

int main(int argc, char** argv)
{
    const char* filename = nullptr;

    Core::ArgsParser args_parser;
    args_parser.add_positional_argument(filename, "Gzip compressed file to decompress", "filename");
    args_parser.parse(argc, argv);

    MappedFile file { filename };

    int rc0, rc1;
    struct timeval before, after;

    {
        rc0 = gettimeofday(&before, nullptr);
        //Core::Gzip::decompress(ByteBuffer::wrap(file.data(), file.size())).value();
        rc1 = gettimeofday(&after, nullptr);
    }

    ASSERT(rc0 == 0 && rc1 == 0);

    struct timeval libcore_delta;
    timersub(&after, &before, &libcore_delta);

    {
        rc0 = gettimeofday(&before, nullptr);
        Compress::GzipDecompressor::decompress_all({ file.data(), file.size() }).value();
        rc0 = gettimeofday(&after, nullptr);
    }

    ASSERT(rc0 == 0 && rc1 == 0);

    struct timeval libcompress_delta;
    timersub(&after, &before, &libcompress_delta);

    const auto libcore_ms = libcore_delta.tv_sec * 1000ull + libcore_delta.tv_usec / 1000ull;
    const auto libcompress_ms = libcompress_delta.tv_sec * 1000ull + libcompress_delta.tv_usec / 1000ull;

    out() << "libcore: " << libcore_ms << "ms, libcompress: " << libcompress_ms << "ms";
}
