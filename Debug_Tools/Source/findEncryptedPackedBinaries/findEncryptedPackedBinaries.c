#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SIGNATURE 0x45C1417E
#define BUFFER_SIZE 4096

void scan_file(const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        return;
    }

    unsigned char buffer[BUFFER_SIZE];
    size_t bytesRead;
    unsigned int signature;

    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (bytesRead < 4) {
            break; // Ensure we have enough bytes to read a 4-byte signature
        }
        for (size_t i = 0; i < bytesRead - 3; i++) {
            signature = (buffer[i] << 24) | (buffer[i + 1] << 16) | (buffer[i + 2] << 8) | buffer[i + 3];
            if (signature == SIGNATURE) {
                if (filepath[0] == '/' && filepath[1] == '/') {
                    printf("Encrypted binary found in %s\n", filepath + 1);
                } else {
                    printf("Encrypted binary found in %s\n", filepath);
                }
                fclose(file);
                return;
            }
        }
    }
    fclose(file);
}

void scan_directory(const char *dirpath) {
    struct dirent *entry;
    DIR *dir = opendir(dirpath);
    if (!dir) {
        return;
    }

    char path[1024];
    struct stat st;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dirpath, entry->d_name);
        if (lstat(path, &st) == 0) {
            if (S_ISLNK(st.st_mode)) {
                continue; // Skip symbolic links
            } else if (S_ISREG(st.st_mode)) {
                scan_file(path);
            } else if (S_ISDIR(st.st_mode)) {
                scan_directory(path);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    char dirpath[1024];
    strncpy(dirpath, argv[1], sizeof(dirpath) - 1);
    dirpath[sizeof(dirpath) - 1] = '\0';
    
    size_t len = strlen(dirpath);
    if (len > 1 && dirpath[len - 1] == '/') {
        dirpath[len - 1] = '\0';
    }
    
    scan_directory(dirpath);
    return EXIT_SUCCESS;
}
