
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
size_t cum_bytes = 0;
// How many cumulative bytes were read last time
size_t prev_bytes = 0;

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
        // We malloc block_size bytes, then immediately free them. This gets us a pointer to some data but prevents memory leakage
        char *data = (char *)malloc(sizeof(char) * block_size);
        free(data);
        int write_fd = open(FIFO_PATH, O_WRONLY);
        // Continually write data to the pipe
        while (write(write_fd, data, block_size) == block_size) { }
        close(write_fd);
        exit(EXIT_SUCCESS);
    }
    else
    {
        signal(SIGALRM, alarm_handler);
        alarm(1);
        
        // Initialize the read buffer
        char *read_buffer = (char *)malloc(sizeof(char) * block_size);

        int read_fd = open(FIFO_PATH, O_RDONLY);
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