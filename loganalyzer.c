#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

void usage() {
    printf("Usage: loganalyzer [-s] <logfile>\n");
    printf("  -s : summary only\n");
}

int main(int argc, char *argv[]) {
    int summary_only = 0;

    // Parse flags
    int opt;
    while ((opt = getopt(argc, argv, "hs")) != -1) {
        switch (opt) {
            case 'h':
                usage();
                return 0;
            case 's':
                summary_only = 1;
                break;
            default:
                usage();
                return 1;
        }
    }

    // Need at least one argument (log file)
    if (optind >= argc) {
        fprintf(stderr, "Error: No logfile provided.\n");
        usage();
        return 1;
    }

    char *filename = argv[optind];

    // Open log file
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return 2;
    }

    // Get file size
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("Error getting file size");
        close(fd);
        return 2;
    }

    // mmap file
    char *map = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return 2;
    }

    // Counters
    int lines = 0, words = 0, chars = 0;
    int err = 0, warn = 0, info = 0;

    // Parse mapped memory
    for (off_t i = 0; i < sb.st_size; i++) {
        char c = map[i];
        chars++;

        if (c == '\n')
            lines++;

        if ((c == ' ' || c == '\n' || c == '\t') &&
            (i > 0 && map[i-1] != ' ' && map[i-1] != '\n' && map[i-1] != '\t'))
            words++;

        // Count ERROR/WARNING/INFO
        if (i + 5 < sb.st_size && strncmp(&map[i], "ERROR", 5) == 0) err++;
        if (i + 7 < sb.st_size && strncmp(&map[i], "WARNING", 7) == 0) warn++;
        if (i + 4 < sb.st_size && strncmp(&map[i], "INFO", 4) == 0) info++;
    }

    if (summary_only) {
        printf("Summary: %d lines, %d words, %d chars, %d ERR, %d WARN, %d INFO\n",
               lines, words, chars, err, warn, info);
    } else {
        printf("File: %s\n", filename);
        printf("Lines: %d\nWords: %d\nCharacters: %d\n", lines, words, chars);
        printf("ERROR entries: %d\nWARNING entries: %d\nINFO entries: %d\n", err, warn, info);
    }

    munmap(map, sb.st_size);
    close(fd);
    return 0;
}
