
#include "1.h"
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define FORK_IN_CHILD 0
#define FORK_FAILURE -1

#define PIPE_FAIL -1
#define PIPE_SUCCESS 0
#define PIPE_READ 0
#define PIPE_WRITE 1

// How many bytes have been read so far
unsigned int cum_bytes = 0;
// How many cumulative bytes were read last time
unsigned int prev_bytes = 0;

// Increases by a multiple of 10 every increase_step alarms
unsigned int block_size = 1;
unsigned int max_block_size = 10000000;
#define MAX_STEP 5
int increase_step = MAX_STEP;

// Continually read and measure performance in the parent process
char *read_buffer = NULL;

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

        // Find some random data on the heap to write through the pipe
        char *data = (char *)malloc(sizeof(char));
        free(data);

        // Set up alarm handler
        signal(SIGALRM, alarm_handler_child);
        alarm(1);

        // Continually write data to the pipe
        int write_fd = pipe_fds[PIPE_WRITE];
        while (1)
            write(write_fd, data, block_size);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // Close the FD's write end, we don't use it
        close(pipe_fds[PIPE_WRITE]);

        // Initialize the read buffer
        read_buffer = (char *)malloc(sizeof(char) * block_size);

        // Set up alarm handler
        signal(SIGALRM, alarm_handler_parent);
        alarm(1);

        int read_fd = pipe_fds[PIPE_READ];
        while (1)
            cum_bytes += read(read_fd, read_buffer, block_size);
    }
    return 0;
}

void alarm_handler_child(int signum)
{
    if (--increase_step == 0)
    {
        increase_step = MAX_STEP;
        block_size *= 10;
    }
    alarm(1);
}

void alarm_handler_parent(int signum)
{
    // Increase the block size by a factor of 10 every MAX_STEP alarms
    if (--increase_step == 0)
    {
        increase_step = MAX_STEP;
        block_size *= 10;
        cum_bytes = 0;
        prev_bytes = 0;
        read_buffer = realloc(read_buffer, sizeof(char) * block_size);

        printf("Block size increased to %u\n================\n", block_size);
    }

    alarm(1);

    printf("Cumulative bytes:\t%.*f MB\n", 2, cum_bytes / pow(10, 6));
    printf("Bandwidth:\t\t%.*f MB/s\n\n", 2, (cum_bytes - prev_bytes) / pow(10, 6));
    prev_bytes = cum_bytes;
}