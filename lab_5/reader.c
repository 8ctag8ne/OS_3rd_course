#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int mem_fd = -1;

void cleanup(int sig) {
    if (mem_fd != -1) close(mem_fd);
    printf("\nReader terminated.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <pid> <hex address>\n", argv[0]);
        return 1;
    }

    signal(SIGINT, cleanup);  // Handle Ctrl+C

    pid_t pid = atoi(argv[1]);
    unsigned long addr = strtoul(argv[2], NULL, 16);

    char mem_path[256];
    snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", pid);

    mem_fd = open(mem_path, O_RDONLY);
    if (mem_fd == -1) {
        perror("open mem");
        return 1;
    }

    int last_value = -1;
    while (1) {
        int current_value;
        if (pread(mem_fd, &current_value, sizeof(current_value), addr) != sizeof(current_value)) {
            perror("pread");
            break;
        }

        if (current_value != last_value) {
            printf("New value detected: %d\n", current_value);
            last_value = current_value;
        }

        sleep(1);
    }

    close(mem_fd);
    return 0;
}
