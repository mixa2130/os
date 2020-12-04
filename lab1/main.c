#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

void exit_handler(void);

int main(void) {
    puts("Start main\n");
    printf(" The process id in main : %x\n", getpid());
    printf(" The parent process id in main : %x\n", getppid());


    int res = atexit(exit_handler);
    if (res != 0)
        // Function wasn't registered
        puts("Atexit error\n");
    pid_t pid = fork();


    // This part will repeat in each process execution
    switch (pid) {
        case -1:
            // An error has happened
            perror("Fork error");
            exit(1); // out of parent process
        case 0:
            sleep(5);
            pid_t child_pid = getpid();
            pid_t parent_pid = getppid();

            printf(" CHILD: My PID - %x\n", child_pid);
            printf(" CHILD: Parent PID - %x\n", parent_pid);

            exit(0);
        default:
            printf("\nNew pid: %x\n\n", pid);

            // The call was successful.
            printf(" PARENT: My PID: %x\n", getpid());
            printf(" PARENT: My CHILD PID: %x\n", pid);

            int ended_child_process = waitpid(pid, 0, 0);
            printf("\n Process %x completed successfully\n", ended_child_process);
    }

    return 0;
}


void exit_handler(void) {
    puts("\nStart atexit");
    printf("The process id in atexit : %x\n", getpid());
    printf("The parent process id in atexit : %x\n", getppid());
}