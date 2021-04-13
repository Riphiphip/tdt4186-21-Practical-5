
#include "1a.h"
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

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
    int pipe_fds[2];
    if (pipe(pipe_fds) == PIPE_FAIL)
    {
        perror("Failed to create pipe");
        return 1;
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
        // Close the FD's read end, we don't use it
        close(pipe_fds[PIPE_READ]);

        // Find some random data to write through the pipe
        // We malloc BLOCK_SIZE bytes, then immediately free them. This gets us a pointer to some data but prevents memory leakage
        char *data = (char *)malloc(sizeof(char) * BLOCK_SIZE);
        free(data);

        // Continually write data to the pipe
        int write_fd = pipe_fds[PIPE_WRITE];
        while (1)
            write(write_fd, data, BLOCK_SIZE);

        exit(EXIT_SUCCESS);
    }
    else
    {
        // Close the FD's write end, we don't use it
        close(pipe_fds[PIPE_WRITE]);

        // Continually read and measure performance in the parent process
        char *buffer = (char *)malloc(sizeof(char) * BLOCK_SIZE);

        // How many bytes have been read so far
        int cum_bytes = 0;
        int read_fd = pipe_fds[PIPE_READ];
        while (1)
            cum_bytes += read(read_fd, buffer, BLOCK_SIZE);
    }

    return 0;
}