#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    if (pledge("stdio wpath cpath", nullptr) < 0) {
        perror("pledge");
        exit(1);
    }

    int fd = open("example.txt", O_WRONLY | O_CREAT, 0644);

    if (pledge("all -stdio -wpath -cpath", nullptr) < 0) {
        perror("pledge");
        exit(1);
    }

    write(fd, "Hello, World!\n", 14);
}
