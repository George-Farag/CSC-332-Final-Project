#include <iostream>
#include <cstring>
#include <unistd.h>      // fork, pipe, read, write, close
#include <sys/mman.h>    // mmap, shm_open, munmap
#include <sys/stat.h>    // ftruncate, mode constants
#include <fcntl.h>       // O_CREAT, O_RDWR
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    const char* shm_name = "/sharedmempipe_example";
    const size_t shm_size = 4096;

    // Create POSIX shared memory object (the "whiteboard" both processes can access)
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Set size of shared memory
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate");
        return 1;
    }

    // Map shared memory into this process
    void* addr = mmap(nullptr, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    char* shared_buffer = static_cast<char*>(addr);

    // Create pipe (the "doorbell" to signal when data is ready)
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child process: read from shared memory after sync

        // Close write end of pipe
        close(pipefd[1]);

        char signal_byte;
        // Wait for parent to "ring the doorbell" through pipe
        if (read(pipefd[0], &signal_byte, 1) <= 0) {
            perror("read");
            return 1;
        }

        // Child reads the message from the "whiteboard"
        std::cout << "Child: read from shared memory: \""
                  << shared_buffer << "\"\n";

        // Clean up in child
        close(pipefd[0]);
        munmap(shared_buffer, shm_size);

        return 0;
    } else {
        // Parent process: write to shared memory, then signal child

        // Close read end of pipe
        close(pipefd[0]);

        // Parent writes message on the "whiteboard"
        const char* message = "Hello from parent using shared memory!";
        std::strncpy(shared_buffer, message, shm_size - 1);
        shared_buffer[shm_size - 1] = '\0';

        // "Ring the doorbell" to signal child that data is ready
        char signal_byte = 'X';
        if (write(pipefd[1], &signal_byte, 1) == -1) {
            perror("write");
            return 1;
        }

        // Wait for child to finish
        wait(nullptr);

        // Clean up in parent
        close(pipefd[1]);
        munmap(shared_buffer, shm_size);
        shm_unlink(shm_name);

        return 0;
    }
}
