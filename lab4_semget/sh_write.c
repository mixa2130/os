#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/sem.h>


#define SEM_ID 177
#define SEMAPHORE_NAME "sh_write.c"
#define TIME_BUFFER_SIZE_STR 27
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


void destructor(int signal) {
    printf("\nHave a good day\n");
    struct shmid_ds *buf = 0;

    if (shmctl(shm_id, IPC_RMID, buf) == -1) {
        printf("shmctl errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    if (semctl(sem_id, 1, IPC_RMID) == -1) {
        printf("semctl errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    exit(0);
}


int main(void) {
    signal(SIGINT, destructor);


    key_t ipc_key = ftok(SH_FILE, IPC_ID);
    if (ipc_key == -1) {
        printf("key errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    shm_id = shmget(ipc_key, sizeof(sh_value), 0777 | IPC_CREAT);
    if (shm_id == -1) {
        printf("shm id errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    sh_value *forward_value; // Values, that will be sent to the reader

    // Connect shared memory
    forward_value = (sh_value *) shmat(shm_id, NULL, 0);
    if (forward_value == (sh_value *) -1) {
        printf("shmat errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    if (forward_value->pid != 0) {
        printf("Sending process already started, close it to create a new one\n");
        exit(0);
    }



    // It's the right process - can initialize the semaphore and start the loop
    key_t sem_key = ftok(SEMAPHORE_NAME, SEM_ID);
    sem_id = semget(sem_key, 1, 0777 | IPC_CREAT);

    if (sem_id == -1) {
        printf("semget errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }


    while (1) {
        // Semaphore lock will start from the second iteration

        if (semop(sem_id, &sop_lock[0], 2) == -1) {
            // The semaphore is waiting for a READ process
            printf("sem_post errno:%d\n", errno);

            exit(EXIT_FAILURE);
        }

        sh_value tmp_value;

        time_t t = time(NULL);
        strncpy((char *) tmp_value.str, ctime(&t), TIME_BUFFER_SIZE_STR - 1);
        tmp_value.str[TIME_BUFFER_SIZE_STR - 1] = '\0';

        tmp_value.pid = getpid();

        printf("Value has been sent, check READ\n");
        *forward_value = tmp_value;


        sleep(2); /* To see a more detailed difference in
                            time received and local time in READ process*/

    }
}