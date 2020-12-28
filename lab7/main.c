#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define READER_THREADS_NUM 10
#define ARRAY_SIZE 10


pthread_mutex_t threads_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

int counter = 0;

/* In this array will store ASCII symbols
 * relevant to counter number + 64*/
char common_thread_arr[ARRAY_SIZE];


void *writer(void *args) {
    while (counter < ARRAY_SIZE) {
        pthread_mutex_lock(&threads_mutex);
            counter++;
            common_thread_arr[counter] = counter + 65;
            printf("counter updated\n"); // new step

        pthread_cond_wait(&cond_var, &threads_mutex);
        pthread_mutex_unlock(&threads_mutex);

        sleep(rand() % 3);
    }

    pthread_exit(0);
}

void *reader(void *args) {
    while(1){

        pthread_mutex_lock(&threads_mutex);
            printf("Thread: %lx. Common array size: %d\n", (long)pthread_self(), counter);
        pthread_cond_signal(&cond_var);
        pthread_mutex_unlock(&threads_mutex);

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



    pthread_mutex_destroy(&threads_mutex);
    pthread_cond_destroy(&cond_var);
    return 0;
}