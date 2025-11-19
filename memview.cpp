#include <iostream>     
#include <fstream>     
#include <sys/mman.h>   
#include <sys/stat.h>   
#include <fcntl.h>      
#include <unistd.h>     
#include <cstdlib>      
#include <cstdio>       

using namespace std;

// This function prints how to use the program if the user types the wrong input
void print_usage(const char* prog) {
    cout << "Usage: " << prog << " <filename>" << endl;
}

// This function prints the contents of the file in a hex + ASCII format
// It shows memory addresses, raw bytes, and readable characters
void print_memory_view(const char* data, size_t size) {
    const int bytesPerLine = 16;  // print 16 bytes per line

    // Loop through the entire file in chunks of 16 bytes
    for (size_t i = 0; i < size; i += bytesPerLine) {

        // Print the offset (memory address) of the current line
        printf("%08zx  ", i);

        // Print each byte in hex format
        for (int j = 0; j < bytesPerLine; j++) {
            if (i + j < size)
                printf("%02x ", (unsigned char)data[i + j]);  // print byte as hex
            else
                printf("   "); // spacing at end of last line
        }

        printf(" ");

        // Print ASCII characters for readable bytes, or '.' for others
        for (int j = 0; j < bytesPerLine; j++) {
            if (i + j < size) {
                char c = data[i + j];
                printf("%c", (c >= 32 && c <= 126) ? c : '.');
            }
        }

        printf("\n");  
    }
}

// This function prints the process's virtual memory map from /proc/self/maps
// It shows stack, heap, shared libraries, etc.
void print_virtual_memory_maps() {
    cout << "\n========== [ Process Virtual Memory Map (/proc/self/maps) ] ==========\n";

    // Open the memory map file
    ifstream maps("/proc/self/maps");
    if (!maps) {
        cerr << "Error: Unable to read /proc/self/maps\n";
        return;
    }

    // Print each line of the memory map
    string line;
    while (getline(maps, line))
        cout << line << endl;
}

// This function shows shared memory segments
void print_shared_memory_segments() {
    cout << "\n========== [ System Shared Memory Segments (ipcs -m) ] ==========\n";

    // Run the ipcs command and capture its output
    FILE* pipe = popen("ipcs -m 2>/dev/null", "r");

    if (!pipe) {
        cerr << "Error: Unable to run ipcs command.\n";
        return;
    }

    char buffer[256];

    // Read the command output line-by-line and print it
    while (fgets(buffer, sizeof(buffer), pipe))
        cout << buffer;

    pclose(pipe);  // close pipe
}

int main(int argc, char* argv[]) {

    // The user must provide exactly ONE argument: the filename
    if (argc != 2) {
        print_usage(argv[0]);  // show how to use the program
        return 1;
    }

    const char* filename = argv[1];  // file to open

    // Try to open the file in read-only mode
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");  // print system error message
        return 1;
    }

    // Use fstat() to get details about the file (size, permissions, etc.)
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat failed");
        close(fd);
        return 1;
    }

    size_t fileSize = sb.st_size;  // get file size in bytes

    // If file is empty, stop the program
    if (fileSize == 0) {
        cerr << "Error: File is empty.\n";
        close(fd);
        return 1;
    }

    // Map the file into memory so we can read it directly
    char* map = (char*)mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);

    // If mmap failed, handle the error
    if (map == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return 1;
    }

    // Display file size and then print a hex dump of the memory region
    cout << "File size: " << fileSize << " bytes\n" << endl;
    print_memory_view(map, fileSize);

    // Unmap memory and close file
    munmap(map, fileSize);
    close(fd);

    // Print the process's virtual memory map
    print_virtual_memory_maps();

    // Print shared memory segments available on the system
    print_shared_memory_segments();

    return 0;  
}
