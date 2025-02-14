#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define HWAES_DEVICE "/dev/hwaes"
#define IOCTL_CMD 0x6205
#define BUFFER_SIZE 256
#define DEFAULT_INPUT_FILE "/etc/uuid_sign"

bool process_file(const char *filename) {
    int fd, hw_fd;
    char dest[BUFFER_SIZE] = {0};  // Zero-initialize buffer
    size_t bytes_read;

    // Open the input file for reading
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening input file");
        return false;
    }

    // Read up to BUFFER_SIZE bytes from the file into dest
    bytes_read = read(fd, dest, BUFFER_SIZE);
    close(fd);

    if (bytes_read <= 0) {
        fprintf(stderr, "Error reading from input file or file is empty\n");
        return false;
    }

    // Open the hardware AES device
    hw_fd = open(HWAES_DEVICE, O_RDWR);
    if (hw_fd < 0) {
        perror("Error opening " HWAES_DEVICE);
        return false;
    }

	int activationResult = 0; // Create a new integer right before ioctl (mandatory), will be used by the custom kernel driver to store activation result

    // Perform ioctl operation
    if (ioctl(hw_fd, IOCTL_CMD, dest) < 0) {
        perror("Error sending ioctl command");
        close(hw_fd);
        return false;
    }

    close(hw_fd);
    
    return activationResult != 0;
}

int main(int argc, char *argv[]) {
    const char *input_file = (argc > 1) ? argv[1] : DEFAULT_INPUT_FILE;
    printf("Using input file: %s\n", input_file);
    
    bool result = process_file(input_file);
    printf("Result: %s\n", result ? "Device is activated :)" : "Device is not activated :(");

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
