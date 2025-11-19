/*
 * test_errors.cpp - Error Handling Tests for filecrypt
 * Demonstrates proper error checking
 */

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

int main() {
    cout << "Testing Error Handling for filecrypt\n\n";
    
    // Test 1: Opening non-existent file
    cout << "Test 1: Opening non-existent file\n";
    int fd = open("nonexistent_file_12345.txt", O_RDONLY);
    if (fd == -1) {
        perror("Error caught");
    } else {
        close(fd);
    }
    cout << "\n";
    
    // Test 2: Writing to read-only file descriptor
    cout << "Test 2: Writing to read-only file descriptor\n";
    ofstream test_file("readonly_test.txt");
    test_file << "test";
    test_file.close();
    
    fd = open("readonly_test.txt", O_RDONLY);
    if (fd != -1) {
        char data = 'X';
        if (write(fd, &data, 1) == -1) {
            perror("Error caught");
        }
        close(fd);
    }
    unlink("readonly_test.txt");
    cout << "\n";
    
    // Test 3: Reading from invalid file descriptor
    cout << "Test 3: Reading from invalid file descriptor\n";
    char buffer[10];
    if (read(-1, buffer, 10) == -1) {
        perror("Error caught");
    }
    cout << "\n";
    
    // Test 4: Getting stats on non-existent file
    cout << "Test 4: fstat on non-existent file\n";
    struct stat st;
    if (fstat(9999, &st) == -1) {
        perror("Error caught");
    }
    cout << "\n";
    
    // Test 5: Creating file in non-existent directory
    cout << "Test 5: Creating file in non-existent directory\n";
    fd = open("/nonexistent_dir_xyz/file.txt", O_WRONLY | O_CREAT, 0600);
    if (fd == -1) {
        perror("Error caught");
    } else {
        close(fd);
    }
    cout << "\n";
    
    cout << "Error handling tests complete\n";
    return 0;
}