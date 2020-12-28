#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define READER_THREADS_NUM 10
#define ARRAY_SIZE 10


pthread_rwlock_t rwlock1 = PTHREAD_RWLOCK_INITIALIZER;
int counter = 0;

/* In this array will store ASCII symbols
 * relevant to counter number + 64*/
char common_thread_arr[ARRAY_SIZE];


void *writer(void *args) {
    while (counter < ARRAY_SIZE) {

        pthread_rwlock_wrlock(&rwlock1);
            counter++;
            common_thread_arr[counter] = counter + 65;
            printf("counter updated\n"); // new step

        pthread_rwlock_unlock(&rwlock1);

        sleep(rand() % 3);
    }

    pthread_exit(0);
}

void *reader(void *args) {
    while(1){
        pthread_rwlock_rdlock(&rwlock1);
            printf("Thread: %lx. Common array size: %d\n", (long)pthread_self(), counter);
        pthread_rwlock_unlock(&rwlock1);

        if (counter == ARRAY_SIZE)
            pthread_exit(0);


        sleep(1);
    }
}


int main(void) {
    pthread_t reader_threads[READER_THREADS_NUM];
    pthread_t writer_thread;


    pthread_create(&writer_thread, NULL, writer, NULL);
    for (int i = 0; i < READER_THREADS_NUM; i++)
        pthread_create(&reader_threads[i], NULL, reader, NULL);



    for (int i = 0; i< READER_THREADS_NUM; i++)
        pthread_join(reader_threads[i], NULL);
    pthread_join(writer_thread, NULL);

    printf("\nIn a galaxy far far away...\nTHE END\n");



    pthread_rwlock_destroy(&rwlock1);

    return 0;
}
