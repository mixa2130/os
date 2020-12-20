#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>

int main() {
    int pipefd[2];  // pipefd[0] - read end
                    // pipefd[1] - write end

    if (pipe(pipefd) == -1) {
        perror("pipe");
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
            close(pipefd[1]); // unused write end

            sleep(5);
            long int child_pipe_buf[2];

            time_t child_time = time(NULL);
            printf("Hello!\nChild time: %s",
                   ctime(&child_time));

            read(pipefd[0], &child_pipe_buf[0], sizeof(long int));
            printf("Parent time: %s",
                   ctime(&child_pipe_buf[0]));

            read(pipefd[0], &child_pipe_buf[1], sizeof(long int));
            printf("\nParent pid: %lu\n", child_pipe_buf[1]);

            close(pipefd[0]);
            exit(0);
        default:
            //parent process
            close(pipefd[0]); // closes unused read end

            long int parent_pipe_buf[2];
            parent_pipe_buf[0] = time(NULL);
            parent_pipe_buf[1] = getpid();

            write(pipefd[1], parent_pipe_buf, sizeof parent_pipe_buf);
            close(pipefd[1]);

            if (waitpid(pid, 0, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
    }

    return 0;
}
