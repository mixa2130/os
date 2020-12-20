#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FIFO_PATH "fifo01"


int main() {
    unlink(FIFO_PATH);

    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("fifo");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    switch (pid) {
        case -1:
            // An error has happened
            perror("Fork");
            exit(EXIT_FAILURE); // out of parent process
        case 0:
            // child process
            puts("Hello!");

            int fd_fifo_chl = open(FIFO_PATH, O_RDONLY);
            if (fd_fifo_chl == -1) {
                fprintf(stderr, "Невозможно открыть fifo\n");
                exit(EXIT_FAILURE);
            }

            long int child_pipe_buf[2];
            if (read(fd_fifo_chl, &child_pipe_buf[0], sizeof(long int)) == -1 ||
                read(fd_fifo_chl, &child_pipe_buf[1], sizeof(long int)) == -1) {
                fprintf(stderr, "Невозможно считать с fifo\n");
                exit(EXIT_FAILURE);
            }

            sleep(5);
            time_t child_time = time(NULL);
            printf("\nChild time: %s", ctime(&child_time));

            printf("Parent time: %s",
                   ctime(&child_pipe_buf[0]));
            printf("\nParent pid: %lu\n", child_pipe_buf[1]);

            close(fd_fifo_chl);
            exit(0);
        default:
            //parent process
            ;
            int fd_fifo_prt = open(FIFO_PATH, O_WRONLY);
            if (fd_fifo_prt == -1) {
                fprintf(stderr, "Невозможно открыть fifo\n");
                exit(EXIT_FAILURE);
            }

            long int parent_pipe_buf[2];
            parent_pipe_buf[0] = time(NULL);
            parent_pipe_buf[1] = getpid();

            write(fd_fifo_prt, parent_pipe_buf, sizeof parent_pipe_buf);
            close(fd_fifo_prt);

            if (waitpid(pid, 0, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
    }

    return 0;
}