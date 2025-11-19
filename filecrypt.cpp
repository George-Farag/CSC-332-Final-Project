#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <getopt.h>
#include <signal.h>
#include <termios.h>

using namespace std;

#define BUFFER_SIZE 4096

volatile sig_atomic_t interrupted = 0; // signal handling

// Secure memory wipe
void secure_wipe(void *ptr, size_t len) {
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) {
        *p++ = 0;
    }
}

// Secure wipe for strings
void secure_wipe_string(string& str) {
    if (!str.empty()) {
        volatile char* p = &str[0];
        for (size_t i = 0; i < str.length(); i++) {
            p[i] = 0;
        }
    }
    str.clear();
}

// Signal handler for interrupts
void handle_signal(int sig) {
    interrupted = 1;
}

// Read password 
string read_password() {
    struct termios old_term, new_term;
    string password;
    
    // Get current terminal settings and try to disable echo
    if (tcgetattr(STDIN_FILENO, &old_term) != 0) {
        cout << "Enter password: ";
        getline(cin, password);
        return password;
    }
    
    // Disable echo
    new_term = old_term;
    new_term.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    
    cout << "Enter password: ";
    cout.flush();
    getline(cin, password);
    cout << endl;
    
    // Restore terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    
    return password;
}

// Print usage
void print_usage(const char* program) {
    cout << "\nUsage: " << program << " [OPTIONS]\n\n";
    cout << "Required:\n";
    cout << "  -i <file>       Input file\n";
    cout << "  -o <file>       Output file\n\n";
    cout << "Mode:\n";
    cout << "  -e              Encrypt (default)\n";
    cout << "  -d              Decrypt\n\n";
    cout << "Optional:\n";
    cout << "  -p <password>   Password (will prompt if not given)\n";
    cout << "  -h              Show help\n\n";
    cout << "Examples:\n";
    cout << "  " << program << " -e -i plain.txt -o encrypted.bin\n";
    cout << "  " << program << " -d -i encrypted.bin -o plain.txt -p mypass\n\n";
}

// XOR encryption/decryption
int xor_crypt(const char* input, const char* output, const string& key) {
    if (key.empty()) {
        cerr << "Error: Password cannot be empty!" << endl;
        return 1;
    }

    // Open input file
    int in_fd = open(input, O_RDONLY);
    if (in_fd < 0) {
        perror("Error opening input file");
        return 1;
    }

    // Get file size
    struct stat st;
    if (fstat(in_fd, &st) < 0) {
        perror("Error getting file info");
        close(in_fd);
        return 1;
    }

    // Create output file
    int out_fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (out_fd < 0) {
        perror("Error creating output file");
        close(in_fd);
        return 1;
    }

    // Process file
    unsigned char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    size_t key_index = 0;

    while ((bytes_read = read(in_fd, buffer, BUFFER_SIZE)) > 0) {
        // Check for interrupt
        if (interrupted) {
            cout << "\nInterrupted! Cleaning up..." << endl;
            secure_wipe(buffer, BUFFER_SIZE);
            close(in_fd);
            close(out_fd);
            unlink(output);
            return 1;
        }

        // XOR each byte
        for (ssize_t i = 0; i < bytes_read; i++) {
            buffer[i] ^= key[key_index % key.length()];
            key_index++;
        }

        // Write output
        if (write(out_fd, buffer, bytes_read) != bytes_read) {
            perror("Error writing output");
            secure_wipe(buffer, BUFFER_SIZE);
            close(in_fd);
            close(out_fd);
            unlink(output);
            return 1;
        }
    }

    // Check for read errors
    if (bytes_read < 0) {
        perror("Error reading input");
        secure_wipe(buffer, BUFFER_SIZE);
        close(in_fd);
        close(out_fd);
        unlink(output);
        return 1;
    }

    // Cleanup
    secure_wipe(buffer, BUFFER_SIZE);
    close(in_fd);
    close(out_fd);

    return 0;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, handle_signal);

    bool encrypt_mode = true;
    string input_file;
    string output_file;
    string password;

    // Parse options
    int opt;
    while ((opt = getopt(argc, argv, "edi:o:p:h")) != -1) {
        switch (opt) {
            case 'e':
                encrypt_mode = true;
                break;
            case 'd':
                encrypt_mode = false;
                break;
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'p':
                password = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (input_file.empty() || output_file.empty()) {
        cerr << "Error: Input and output files required!\n" << endl;
        print_usage(argv[0]);
        return 1;
    }

    if (input_file == output_file) {
        cerr << "Error: Input and output cannot be the same!" << endl;
        return 1;
    }

    if (password.empty()) {
        password = read_password();
        if (password.empty()) {
            cerr << "Error: Password cannot be empty!" << endl;
            return 1;
        }
    }

    cout << (encrypt_mode ? "Encrypting" : "Decrypting") << "..." << endl;
    int result = xor_crypt(input_file.c_str(), output_file.c_str(), password);

    // Wipe password
    secure_wipe_string(password);

    if (result == 0) {
        cout << "Done!" << endl;
    } else {
        cerr << "Failed!" << endl;
    }

    return result;
}