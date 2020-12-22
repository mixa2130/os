#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define READER_THREADS_NUM 10
#define ARRAY_SIZE 10


pthread_mutex_t threads_mutex = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

/* In this array will store ASCII symbols
 * relevant to counter number + 65*/
char common_thread_arr[ARRAY_SIZE];


void *writer(void *args) {
    while (counter < ARRAY_SIZE) {

        pthread_mutex_lock(&threads_mutex);
            common_thread_arr[counter++] = counter + 65;
            printf("counter:%d\n", counter); // new step
        pthread_mutex_unlock(&threads_mutex);

        sleep(1);
    }

//    printf("Write thread: %lu has ended\n", pthread_self());
    pthread_exit(0);
}

void *reader(void *args) {
    while(1){
        pthread_mutex_lock(&threads_mutex);
            printf("Thread: %lu. Common array: ", pthread_self());

            for (int i = 0; i < counter; i++)
                printf("%c", common_thread_arr[i]);
            printf("\n");

        pthread_mutex_unlock(&threads_mutex);

        if (counter == ARRAY_SIZE) {
//            pthread_mutex_lock(&threads_mutex);
//            printf("Thread: %lu has ended\n", pthread_self());
//            pthread_mutex_unlock(&threads_mutex);
            pthread_exit(0);
        }

        sleep(1);


    }
}


int main() {
    pthread_t reader_threads[READER_THREADS_NUM];
    pthread_t writer_thread;

    for (int i = 0; i < READER_THREADS_NUM; i++)
        pthread_create(&reader_threads[i], NULL, reader, NULL);
    pthread_create(&writer_thread, NULL, writer, NULL);

    for (int i = 0; i< READER_THREADS_NUM; i++) {
        pthread_join(reader_threads[i], NULL);
    }
    pthread_join(writer_thread, NULL);

    printf("In a galaxy far far away...\nTHE END");
    pthread_mutex_destroy(&threads_mutex);
    return 0;
}
