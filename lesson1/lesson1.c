#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

typedef struct thread_data {
	long threadid;
	char *message;
} thread_data_t;

void *thread(void *data) {
	thread_data_t *casted_data = (thread_data_t *)data;
	printf("From inside: { .threadid = %ld, .message = \"%s\" }\n", casted_data->threadid, casted_data->message);
	pthread_exit(NULL);
}

int main(void) {
	int returnval;
	pthread_t tid;


	thread_data_t *data = (thread_data_t *)malloc(sizeof(thread_data_t));
	data->threadid = 10;
	data->message = "Hello, Amy";

	returnval = pthread_create(&tid, NULL, thread, data);

	printf("returnval: %d\n", returnval);

	int test = 0;
	while (test++ < 100000) {}

	printf("Pthreads example!\n");

	pthread_join(tid, NULL);

	return 0;
}

