#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    std::cout << "Testing Error Handling\n\n";
    
    // Test 1: Invalid shared memory name
    std::cout << "Test 1: Invalid shared memory name\n";
    int shm_fd = shm_open("", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Error caught");
    }
    std::cout << "\n";
    
    // Test 2: Writing to closed pipe
    std::cout << "Test 2: Writing to closed pipe\n";
    int pipefd[2];
    pipe(pipefd);
    close(pipefd[1]);
    
    char data = 'X';
    if (write(pipefd[1], &data, 1) == -1) {
        perror("Error caught");
    }
    close(pipefd[0]);
    std::cout << "\n";
    
    std::cout << "Tests complete\n";
    return 0;
}
