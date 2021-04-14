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

/* 
 * The largest block size that worked on my system was 133 456 bytes.
 * 
 * Among the powers of 10, bandwidth was the greatest when transferring 100 000 bytes.
 * This gave a bandwidth of ~1400MB/s, or 1.4GB/s
 * 
 * Running 2 programs transferring blocks of 10 bytes and 2 programs transferring blocks of 100 000 bytes simulatenously.
 * The bandwidth surprisingly went up for the processes transferring 100 000 bytes, peaking at ~2.6GB/s
 * The bandwidth on the programs transfering 10 bytes, however, went down from ~12MB/s to ~8MB/s
 */

int main(int argc, char *argv[])
{
    size_t block_size = 1;
    if (argc > 1)
    {
        char *trash;
        block_size = strtoul(argv[1], &trash, 0);
    }
    printf("Measuring bandwidth with block size %lu bytes\n", block_size);

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

        // Continually write data to the pipe
        int write_fd = pipe_fds[PIPE_WRITE];
        while (write(write_fd, data, block_size) == block_size) {}
        close(write_fd);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // Close the FD's write end, we don't use it
        close(pipe_fds[PIPE_WRITE]);

        // Initialize the read buffer
        char *read_buffer = (char *)malloc(sizeof(char) * block_size);

        // Set up alarm handler
        signal(SIGALRM, alarm_handler);
        alarm(1);

        int read_fd = pipe_fds[PIPE_READ];
        size_t bytes_read = 0;
        do
        {
            // not as fast as possible, but necessary to detect when to stop reading
            bytes_read = read(read_fd, read_buffer, block_size);
            cum_bytes += bytes_read;
        } while (bytes_read > 0);
        close(read_fd);
    }
    return 0;
}

void alarm_handler(int signum)
{
    alarm(1);

    printf("Cumulative bytes:\t%.*f MB\n", 2, cum_bytes / pow(10, 6));
    printf("Bandwidth:\t\t%.*f MB/s\n\n", 2, (cum_bytes - prev_bytes) / pow(10, 6));
    prev_bytes = cum_bytes;
}