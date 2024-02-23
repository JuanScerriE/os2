#include <errno.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

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

#define MSG_LEN 32

typedef struct thread_data {
  char message[MSG_LEN];
} thread_data_t;

void *thread(void *arg) {
  thread_data_t *tdata = (thread_data_t *)arg;

  printf("%s\n", tdata->message);

  return 0;
}

int main(void) {
  int errnum;

  pthread_t tid;

  pthread_attr_t attr;

  pthread_attr_init(&attr);

  // this basically makes such that the whole Thread Control
  // Block is freed immediately after completion
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  thread_data_t tdata;

  snprintf(tdata.message, MSG_LEN, "Thread: Ocean Drive");

  if ((errnum = pthread_create(&tid, &attr, thread, &tdata)) != 0) {
    handle_pthread_error(errnum);

    exit(EXIT_FAILURE);
  }

  sleep(5); // sleep to give the thread a chance to finish

  pthread_attr_destroy(&attr);

  return 0;
}
