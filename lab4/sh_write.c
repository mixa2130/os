#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>

#define TIME_BUFFER_SIZE_STR 27
#define IPC_ID 77
#define SEMAPHORE_NAME "/sem235"
#define SH_FILE "sh_read.c" // Here can be absolutely any exist file,
                            // cause we just create a unique id for shared memory


typedef struct Value {
    char str[TIME_BUFFER_SIZE_STR]; // Process time
    pid_t pid;                      // Process pid
} sh_value;

int shm_id;
sem_t *sem; // The semaphore, which will be used in
            // program processes READ and WRITE



void destructor(int signal) {
    printf("\nHave a good day\n");
    struct shmid_ds *buf = 0;
    shmctl(shm_id, IPC_RMID, buf);

    if (sem != NULL &&
        (sem_close(sem) == -1 || sem_unlink(SEMAPHORE_NAME) == -1)) {

        printf("sem destruction errno:%d\n", errno);
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
    sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0777, 0);
    if (sem == SEM_FAILED) {
        printf("sem_open errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    while (1) {
        sh_value tmp_value;

        time_t t = time(NULL);
        strncpy((char *) tmp_value.str, ctime(&t), TIME_BUFFER_SIZE_STR - 1);
        tmp_value.str[TIME_BUFFER_SIZE_STR - 1] = '\0';

        tmp_value.pid = getpid();

        printf("Value has been sent, check READ\n");
        *forward_value = tmp_value;

        if (sem_wait(sem) == -1) { // Turning on the semaphore
            printf("sem_post errno:%d\n", errno);
            exit(EXIT_FAILURE);
        }

        sleep(3); // To see a more detailed difference in
                         // time received and local time in READ process
    }
}