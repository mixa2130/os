#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>


#define TIME_BUFFER_SIZE_STR 25
#define SH_FILE "sh_read.c" // Here can be absolutely any exist file,
                            // cause we just create a unique id for shared memory
#define IPC_ID 77
#define SEMAPHORE_NAME "/sem235"

typedef struct Value {
    char str[TIME_BUFFER_SIZE_STR]; // Process time
    pid_t pid;                      // Process pid
} sh_value;

int shm_id;
sem_t *sem;

void sh_memory_destructor(int signal)
{
    printf("\nKeep calm and code on!\n");

    if(sem != NULL && sem_close(sem) == -1){
        printf("sem_close errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    exit(0); // Shared memory segments will be undocked
}

int main(void) {
    signal(SIGINT, sh_memory_destructor);


    key_t ipc_key = ftok(SH_FILE, IPC_ID);
    if (ipc_key == -1) {
        printf("key errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }


    shm_id = shmget(ipc_key, sizeof(sh_value), 0777 | IPC_EXCL);
    if (shm_id == -1) {
        printf("shm id errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Connect shared memory
    sh_value *received_value = (sh_value *) shmat(shm_id, NULL, 0);
    if (received_value == (sh_value *) -1) {
        printf("shmat errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }


    sem = sem_open(SEMAPHORE_NAME, 0);
    if(sem == SEM_FAILED){
        printf("sem_open errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    while (1) {
        sh_value local_value;

        time_t local_time = time(NULL);
        strncpy((char *) local_value.str, ctime(&local_time), TIME_BUFFER_SIZE_STR - 1);
        local_value.str[TIME_BUFFER_SIZE_STR-1] = '\0';
        local_value.pid = getpid();


        printf("my pid:%d my time:%s \n", local_value.pid, local_value.str);
        printf("Data read from memory: %d %s\n", received_value->pid, received_value->str);

        if(sem_post(sem) == -1){ // Semaphore is waiting fo a WRITE process
            printf("sem_post errno:%d\n", errno);
            exit(EXIT_FAILURE);
        }

        sleep(3); // To see a more detailed difference in
        // time received and local time in READ process
    }
}