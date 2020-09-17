#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    if (pledge("", nullptr) < 0) {
        perror("pledge");
        exit(1);
    }

    FILE* fp = fopen("example.txt", "w");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    if (pledge("all -rpath -stdio", nullptr) < 0) {
        perror("pledge");
        exit(1);
    }

    fwrite("foo", 3, 1, fp);
    fclose(fp);
}
