#include <chrono>

#include <AK/BitStream.h>
#include <AK/MemoryStream.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

template<typename Callable>
static std::chrono::nanoseconds benchmark(StringView name, Callable callback)
{
    outf("benchmark: {}", name);

    const auto before = std::chrono::high_resolution_clock::now();
    callback();
    const auto after = std::chrono::high_resolution_clock::now();

    outf("completed in {}ns ({}ms)", (after - before).count(), std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count());

    return after - before;
}

static ReadonlyBytes map_file_into_memory(const char* filename)
{
    int fd = open(filename, O_RDONLY);

    struct stat statbuf;
    fstat(fd, &statbuf);

    const auto* memory = mmap(nullptr, statbuf.st_size, PROT_READ, MAP_ANONYMOUS, fd, 0);

    close(fd);

    outf("read {} bytes ({}MiB) from {}", statbuf.st_size, statbuf.st_size / (1024 * 1024), filename);

    return { memory, static_cast<size_t>(statbuf.st_size) };
}

int main()
{
    auto buffer = map_file_into_memory("/home/me/Documents/gzip-1.10.tar");

    {
        InputMemoryStream stream { buffer };
        InputBitStream bitstream { stream };

        benchmark("bit_by_bit", [&bitstream]() {
            [[maybe_unused]] volatile bool bit = bitstream.read_bit();
        });
        bitstream.handle_any_error();
        stream.handle_any_error();
    }

    {
        InputMemoryStream stream { buffer };
        InputBitStream bitstream { stream };

        benchmark("three_then_five", [&bitstream]() {
            [[maybe_unused]] volatile int value0 = bitstream.read_bits(3);
            [[maybe_unused]] volatile int value1 = bitstream.read_bits(5);
        });
        bitstream.handle_any_error();
        stream.handle_any_error();
    }
}
