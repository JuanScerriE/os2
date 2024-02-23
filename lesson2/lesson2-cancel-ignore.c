#include <errno.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>

#define NUM_OF_THREADS 4

#define ERROR_BUF_LEN 256

static inline void handle_pthread_error(int errnum) {
  char strerrbuf[ERROR_BUF_LEN];

  if (errnum < sys_nerr) {
    if (strerror_r(errnum, strerrbuf, ERROR_BUF_LEN) == ERANGE) {
      fprintf(stderr,
              "pthread_create: could not create the thread, error number "
              "[%d] - %s (insufficient error buffer size)\n",
              errnum, strerrbuf);
    } else {
      fprintf(stderr,
              "pthread_create: could not create the thread, error number "
              "[%d] - %s\n",
              errnum, strerrbuf);
    }
  } else {
    fprintf(stderr,
            "pthread_create: could not create the thread, error number "
            "[%d] (%d > sys_nerr)\n",
            errnum, errnum);
  }
}

#define MSG_LEN 16

typedef struct thread_data {
  char message[MSG_LEN];
  int number_of_iter;
} thread_data_t;

void *thread(void *arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  sleep(1);

  // NOTE: the string "Hello, Chairman Of The Malta Gaming Authority!"
  // is actually allocated on the stack of the function which
  // started the thread in our case the main function.
  thread_data_t *tdata = (thread_data_t *)arg;

  for (int i = 0; i < tdata->number_of_iter; i++) {
    printf("%s\n", tdata->message);
  }

  return 0;
}

#define ITER_MULTIPLIER 10

int main(void) {
  int errnum;

  pthread_t tids[NUM_OF_THREADS];

  thread_data_t tdatas[NUM_OF_THREADS];

  for (int i = 0; i < NUM_OF_THREADS; i++) {
    snprintf(tdatas[i].message, MSG_LEN, "Thread %d", i);

    tdatas[i].number_of_iter = ITER_MULTIPLIER * (i + 1);

    if ((errnum = pthread_create(&tids[i], NULL, thread, &tdatas[i])) != 0) {
      handle_pthread_error(errnum);

      exit(EXIT_FAILURE);
    }
  }

  srand(time(NULL));

  unsigned int random_thread = rand() % NUM_OF_THREADS;

  if ((errnum = pthread_cancel(tids[random_thread])) != 0) {
    // we need to make sure to handle this case because the
    // thread might not actually be alive anymore especially if
    // we detach.
    handle_pthread_error(errnum);
  }

  void *return_value;

  for (int i = 0; i < NUM_OF_THREADS; i++) {
    if ((errnum = pthread_join(tids[i], &return_value)) != 0) {
      handle_pthread_error(errnum);

      exit(EXIT_FAILURE);
    }

    if (return_value == PTHREAD_CANCELED) {
      printf("pthread_join: thread [%d] had been canceled\n", i);
    }
  }

  return 0;
}
