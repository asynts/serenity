#include <unistd.h>
#include <stdio.h>

int main() {
    if(pledge("", nullptr) < 0) {
        fprintf(stderr, "pledge() returned non zero\n");
        return 1;
    }

    execlp("echo", "echo", "execlp was executed", nullptr);
}
