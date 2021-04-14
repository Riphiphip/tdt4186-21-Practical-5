
#include "1.h"

#define FORK_IN_CHILD 0
#define FORK_FAILURE -1

#define PIPE_FAIL -1
#define PIPE_SUCCESS 0
#define PIPE_READ 0
#define PIPE_WRITE 1

// allow specifying the block size on compile
#ifndef BLOCK_SIZE
#define BLOCK_SIZE 100
#endif

int main()
{
    // Create the pipe
    unlink(FIFO_PATH);
    int result = mkfifo(FIFO_PATH, 0666);
    if (result){
        perror("Error while creating named pipe");
        return -1;
    }

    // Fork off a new child
    pid_t child_pid = fork();
    if (child_pid == FORK_FAILURE)
    {
        perror("Failed to create child process");
        return 1;
    }

    if (child_pid == FORK_IN_CHILD)
    {
        // Find some random data to write through the pipe
        // We malloc BLOCK_SIZE bytes, then immediately free them. This gets us a pointer to some data but prevents memory leakage
        char *data = (char *)malloc(sizeof(char) * BLOCK_SIZE);
        free(data);
        int write_fd = open(FIFO_PATH, O_WRONLY);
        // Continually write data to the pipe
        while (write(write_fd, data, BLOCK_SIZE) > 0)
        {
        }
        exit(EXIT_SUCCESS);
    }
    else
    {
        // Continually read and measure performance in the parent process
        char *buffer = (char *)malloc(sizeof(char) * BLOCK_SIZE);

        // How many bytes have been read so far
        int cum_bytes = 0;
        int read_fd = open(FIFO_PATH, O_RDONLY);
        do
        {
            cum_bytes += read(read_fd, buffer, BLOCK_SIZE);
            printf("Cumulative bytes read: %d\n", cum_bytes);
        } while (cum_bytes > 0);
    }

    return 0;
}