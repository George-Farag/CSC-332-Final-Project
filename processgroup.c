#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#define MAX_CHILDREN 20

// array holding the PIDs of all children
int child_pids[MAX_CHILDREN];

// default number of children unless -n is run
int num_children = 5;

// flags for optional program modes
int quiet_mode = 0;      // Suppress child output
int random_mode = 0;     // Each child sleeps random time (1-3 seconds)

// flag modified by SIGINT handler to trigger cleanup
volatile sig_atomic_t stop_flag = 0;

// sets the stop_flag so the main loop knows it's time to shut down.
void sigint_handler(int sig) {
    (void)sig; // unused parameter
    stop_flag = 1;
    write(STDOUT_FILENO, "\n[processgroup] SIGINT caught — cleaning up...\n", 47);
}

// creates 'count' child processes using fork()
// each child prints messages unless quiet mode is enabled
void spawn_children(int count) {
    for (int i = 0; i < count; i++) {

        pid_t pid = fork();

        if (pid < 0) {
            // fork failed
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            if (!quiet_mode)
                printf("[Child %d] Started (PID=%d)\n", i, getpid());

            // if random mode is enabled, children run at different speeds
            int delay = random_mode ? (rand() % 3) + 1 : 1;

            // loops until parent kills child with sigterm
            while (1) {
                if (!quiet_mode)
                    printf("[Child %d] Running...\n", i);
                sleep(delay);
            }
            exit(0);
        } else {  
            child_pids[i] = pid;  
        }
    }
}

// sends sigterm to kill each child one at a time
void kill_children(int count) {
    for (int i = 0; i < count; i++) {
        if (child_pids[i] > 0) {
            kill(child_pids[i], SIGTERM);
        }
    }
}

// waits for all children to finish, preventing zombies
void wait_for_children(int count) {
    for (int i = 0; i < count; i++) {
        if (child_pids[i] > 0) {
            waitpid(child_pids[i], NULL, 0);
        }
    }
}

// prints directions for user testing
void print_usage() {
    printf("Usage: processgroup [options]\n");
    printf("Options:\n");
    printf("  -n <num>     Number of children (1-%d)\n", MAX_CHILDREN);
    printf("  -q           Quiet mode (children do not print)\n");
    printf("  -r           Random delay for children\n");
    printf("  --test       Run built-in test cases\n");
}

// prints a set of manual test cases for demonstration
void run_test_cases() {
    printf("\n Test Cases \n");

    printf("\nTEST 1: Run default (5 children)\n");
    printf("TEST 2: Invalid -n value\n");
    printf("TEST 3: -n 3 spawns exactly 3 children\n");
    printf("TEST 4: -q quiet mode suppresses child prints\n");
    printf("TEST 5: -r randomizes child timing\n");
    printf("TEST 6: Ctrl+C results in clean shutdown\n");
    printf("TEST 7: Stress test with -n 20\n");

}

// main program
int main(int argc, char *argv[]) {

    srand(time(NULL)); // Seed for random mode

    // command-line arguments :)
    for (int i = 1; i < argc; i++) {

        // Set number of children
        if (!strcmp(argv[i], "-n") && i + 1 < argc) {
            num_children = atoi(argv[++i]);
            if (num_children <= 0 || num_children > MAX_CHILDREN) {
                fprintf(stderr, "Error: -n value must be between 1 and %d\n", MAX_CHILDREN);
                return 1;
            }
        }
        // Quiet mode
        else if (!strcmp(argv[i], "-q")) {
            quiet_mode = 1;
        }
        // Random mode
        else if (!strcmp(argv[i], "-r")) {
            random_mode = 1;
        }
        // Show test cases and exit
        else if (!strcmp(argv[i], "--test")) {
            run_test_cases();
            return 0;
        }
        else {
            print_usage();
            return 1;
        }
    }

    // Startup message
    printf("[processgroup] Starting with %d child processes.\n", num_children);
    if (quiet_mode)  printf("[processgroup] Quiet mode enabled.\n");
    if (random_mode) printf("[processgroup] Random mode enabled.\n");

    // install handler for the ctrl+c input
    signal(SIGINT, sigint_handler);

    // creates children
    spawn_children(num_children);

    // parent sleeps until sigint is received
    while (!stop_flag) {
        sleep(1);
    }

    // cleanup phase
    printf("[processgroup] Terminating children...\n");
    kill_children(num_children);

    printf("[processgroup] Waiting for children to exit...\n");
    wait_for_children(num_children);

    printf("[processgroup] All cleaned up. Exiting.\n");
    return 0;
}
