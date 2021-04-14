#pragma once

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/fifo-pipe-for-task-1d-please-do-not-mess-with-this-file.pipe"

void alarm_handler(int signum);
void usr1_handler(int signum);