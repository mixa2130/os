#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

#define TIME_BUFFER_SIZE_STR 27
#define SH_FILE "sh_read.c" // Here can be absolutely any exist file,
                            // cause we just create a unique id for shared memory
#define IPC_ID 77

typedef struct Value {
    char str[TIME_BUFFER_SIZE_STR]; // Process time
    pid_t pid;                      // Process pid
} sh_value;

int shm_id;

void sh_memory_destructor(int signal){
    printf("Have a good day\n");
    struct shmid_ds *buf = 0;
    shmctl(shm_id, IPC_RMID, buf);

    exit(0);
}


int main(void) {
    signal(SIGINT, sh_memory_destructor);

    key_t ipc_key = ftok(SH_FILE, IPC_ID);
    if (ipc_key == -1) {
        printf("key errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    shm_id = shmget(ipc_key, sizeof(sh_value), 0666 | IPC_CREAT);
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

    if(forward_value->pid != 0){
        printf("Sending process already started, close it to create a new one\n");
        exit(0);
    }

    while (1) {
        sh_value tmp_value;

        time_t t = time(NULL);
        strncpy((char *) tmp_value.str, ctime(&t), TIME_BUFFER_SIZE_STR - 1);
        tmp_value.str[TIME_BUFFER_SIZE_STR-1] = '\0';

        tmp_value.pid = getpid();

        printf("Value has been sent, check READ\n");
        *forward_value = tmp_value;

        sleep(2); // To see a more detailed difference in
                         // time received and local time in READ process
    }
}