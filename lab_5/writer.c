#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main() {
    static volatile int value = 0;

    pid_t pid = getpid();
    printf("Writer PID: %d\n", pid);
    printf("Address of value: %p\n", (void*)&value);
    printf("To monitor value: sudo ./reader %d %p\n\n", pid, (void*)&value);

    while (1) {
        printf("Enter a number: ");
        fflush(stdout);

        if (scanf("%d", (int*)&value) != 1) {
            printf("Invalid input. Try again.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }

        printf("Updated value to: %d\n", value);
    }

    return 0;
}
