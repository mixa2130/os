#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

void exit_handler(void);

int main(void) {
    puts("Start main");
    printf("The process id in main : %d\n", getpid());
    printf("The parent process id in main : %d\n", getppid());


    int res = atexit(exit_handler);
    if (res != 0)
        // Function wasn't registered
        puts("Atexit error\n");

    pid_t pid;
    printf("\nnew pid : %d\n", pid = fork());

    // This part will repeat in each process execution
    switch (pid) {
        case -1:
            // An error has happened
            perror("Fork error");
            exit(1); // out of parent process
        case 0:
            printf("In child process with parent %d\n", getppid());
            exit(0);
        default:
            // The call was successful.
            // The parent process gets the child's pid
            printf("Wait child in parent process %d\n", pid);

            int ended_child_process = waitpid(pid, 0, 0);
            printf("\nProcess %d completed successfully\n", ended_child_process);
    }

    return 0;
}


void exit_handler(void) {
    puts("\nStart atexit");
    printf("The process id in atexit : %d\n", getpid());
    printf("The parent process id in atexit : %d\n", getppid());
}