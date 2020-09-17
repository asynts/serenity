#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
$ echo foo > example.txt
$ dummy
1 (1 process)
4 (1 process)
5 (1 process)
6 (1 process)
7 (1 process)
8 (1 process)
foo
9 (1 process)
1 (2 process)
2 (2 process)
1 (1 process)
pledge: Operation not permitted
$ echo $?
1
*/

int main(int argc, char**)
{
    fprintf(stderr, "%d (%d process)\n", 1, argc);

    if (argc == 2) {
        fprintf(stderr, "%d (%d process)\n", 2, argc);

        if (pledge("exec", nullptr) < 0) {
            perror("pledge");
            exit(1);
        }

        fprintf(stderr, "%d (%d process)\n", 3, argc);
    }

    fprintf(stderr, "%d (%d process)\n", 4, argc);

    if (pledge("exec stdio rpath", nullptr) < 0) {
        perror("pledge");
        exit(1);
    }

    fprintf(stderr, "%d (%d process)\n", 5, argc);

    int fd = open("example.txt", O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    fprintf(stderr, "%d (%d process)\n", 6, argc);

    if (pledge("all -rpath", nullptr) < 0) {
        perror("pledge");
        exit(1);
    }

    fprintf(stderr, "%d (%d process)\n", 7, argc);

    char buf[64];
    auto length = read(fd, buf, sizeof(buf));

    fprintf(stderr, "%d (%d process)\n", 8, argc);

    printf("%.*s", (int)length, buf);

    fprintf(stderr, "%d (%d process)\n", 9, argc);

    if (argc == 1) {
        pledge(nullptr, "stdio");
        execlp("dummy", "dummy", "foo", nullptr);
    }

    fprintf(stderr, "%d (%d process)\n", 10, argc);
}
