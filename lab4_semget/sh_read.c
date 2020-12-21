#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/sem.h>


#define SEM_ID 177
#define SEMAPHORE_NAME "sh_write.c"
#define TIME_BUFFER_SIZE_STR 25
#define IPC_ID 77
#define SH_FILE "sh_read.c" /*Here can be absolutely any exist file,
                            cause we just create a unique id for shared memory*/


typedef struct Value {
    char str[TIME_BUFFER_SIZE_STR]; // Process time
    pid_t pid;                      // Process pid
} sh_value;


/*
 * If semaphore value = 0 - resource is available
 * If semaphore value = 1 - resource is locked
 * else - something went wrong
 * */
static struct sembuf sop_lock[2] = {

        0, 0, 0, // Waiting for semaphore nullification

        0, 1, 0  // Then increase semaphore value
};

static struct sembuf sop_unlock[1] = {

        0, -1, 0 // Nullify semaphore value
};


int shm_id;
int sem_id;


void sh_memory_destructor(int signal) {
    printf("\nKeep calm and code on!\n");

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


    key_t sem_key = ftok(SEMAPHORE_NAME, SEM_ID);
    sem_id = semget(sem_key, 1, 0777);


    if (sem_id == -1) {
        printf("semget errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    while (1) {

        // Read the first value from memory, sent before semaphore start
        sh_value local_value;

        time_t local_time = time(NULL);
        strncpy((char *) local_value.str, ctime(&local_time), TIME_BUFFER_SIZE_STR - 1);
        local_value.str[TIME_BUFFER_SIZE_STR - 1] = '\0';
        local_value.pid = getpid();


        printf("my pid:%d my time:%s \n", local_value.pid, local_value.str);
        printf("Data read from memory: %d %s\n", received_value->pid, received_value->str);

        if (semop(sem_id, &sop_unlock[0], 1) == -1) {
            printf("sem_post errno:%d\n", errno);
            exit(EXIT_FAILURE);
        }
        sleep(2);

    }
}