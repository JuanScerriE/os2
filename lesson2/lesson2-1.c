#include <errno.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <time.h>
// #include <unistd.h>

#define NUM_OF_THREADS 1

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

// Exercise1

void *thread(void *arg) {
  printf("Hello World!\n");

  return 0;
}

int main(void) {
  int return_value;

  pthread_t tids[NUM_OF_THREADS];

  for (int i = 0; i < NUM_OF_THREADS; i++) {
    if ((return_value = pthread_create(&tids[i], NULL, thread, NULL)) != 0) {
      handle_pthread_error(return_value);

      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < NUM_OF_THREADS; i++) {
    if ((return_value = pthread_join(tids[i], NULL)) != 0) {
      handle_pthread_error(return_value);

      exit(EXIT_FAILURE);
    }
  }

  return 0;
}
