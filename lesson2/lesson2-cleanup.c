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

#define TEXT_LEN 256

typedef struct thread_data {
  char message[MSG_LEN];
} thread_data_t;

void cleanup_thread(void *arg) {
  printf("Welcome to the Human Race!\n");

  if (arg)
    free(arg);
}

void the_oldest_view_thread(void *arg) {
  printf("Julien Reverchon\n");
}

void *thread(void *arg) {
  char *text = NULL;

  pthread_cleanup_push(the_oldest_view_thread, NULL);
  pthread_cleanup_push(cleanup_thread, text);

  text = (char *)malloc(sizeof(char) * TEXT_LEN);

  sleep(1);

  thread_data_t *tdata = (thread_data_t *)arg;

  snprintf(text, TEXT_LEN, "%s - %p", tdata->message, pthread_self());

  printf("%s\n", text);

  // pthread_exit(0); // this should pop all handlers

  // NOTE: pushing and popping handlers is essentially stack
  // behaviour. That is as per our example the first popped
  // function will be cleanup_thread and then
  // the_oldest_view_thread. Additionally, we can manually
  // control which of the handlers are executed. Passing 0 with
  // in the pthread_cleanup_pop function essentially forces the
  // pop to occur without executing the handler anything else
  // will cause the handler to run.
  pthread_cleanup_pop(1);
  pthread_cleanup_pop(0);

  return 0;
}

// we can essentially add a cleanup function which when the
// thread receives a cancel signal it will actually call the
// pushed thread function to do clean up before cancelling.

int main(void) {
  int errnum;

  pthread_t tids[NUM_OF_THREADS];

  thread_data_t tdatas[NUM_OF_THREADS];

  for (int i = 0; i < NUM_OF_THREADS; i++) {
    snprintf(tdatas[i].message, MSG_LEN, "Thread: %d", i);

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
