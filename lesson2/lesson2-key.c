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

  if (strerror_r(errnum, strerrbuf, ERROR_BUF_LEN) == ERANGE) {
    fprintf(stderr,
            "pthread_create: could not create the thread, error number "
            "[%d] - (%s) {insufficient error buffer size}\n",
            errnum, strerrbuf);
  } else {
    fprintf(stderr,
            "pthread_create: could not create the thread, error number "
            "[%d] - (%s)\n",
            errnum, strerrbuf);
  }
}

#define MSG_LEN 128

typedef struct thread_data {
  pthread_t tid;
  int msg_reps;
  char message[MSG_LEN];
} thread_data_t;

// So, we shall proceed to give an explanation of why one might
// use pthread_keys. This is because when someone actually
// thinks a bit about it one might ask why would you actually
// use a pthread_key. Can't you just make use of a local
// thread variable? And then just add a cleanup function which
// can be used to make sure that the allocated data is freed
// even if the thread is cancelled. But I guess the use of keys
// is a bit more convenient. This is because you do not have to
// be careful about calling the destructor. As long as the
// it will always be called. So, I guess it is a bit more
// convenient then other methods. That's its redeeming quality.
//
// Additionally, a key can act as a per thread global variable.
// The same way in which you can create a global variable in a
// single threaded program and its accessible everywhere i.e. in
// any function call. The same goes for keys. They are
// accessible by any function call in a thread, but not by any
// other thread.

pthread_key_t key;

pthread_once_t once_control = PTHREAD_ONCE_INIT;

void generate_random_msg(char *msg, size_t size) {
  if (size > MSG_LEN) {
    return;
  }

  for (size_t i = 0; i < size - 1; i++) {
    msg[i] = (rand() % 95) + 32;
  }

  msg[size - 1] = 0;
}

void create_thread_data(void) {
  thread_data_t *tdata = (thread_data_t *)malloc(sizeof(thread_data_t));

  printf("ALLOCATING DATA AT %p...\n", tdata);

  tdata->msg_reps = (rand() % 20) + 1;
  generate_random_msg(tdata->message, 16);
  tdata->tid = pthread_self();

  int errnum;

  if ((errnum = pthread_setspecific(key, tdata)) != 0) {
    handle_pthread_error(errnum);

    exit(EXIT_FAILURE);
  }
}

void free_keyed(void *data) {
  printf("FREEING KEY DATA %p...\n", data);

  thread_data_t *tdata = (thread_data_t *)data;

  if (tdata)
    free(tdata);
}

// NOTE: You shouldn't do this. The only reasonable place to
// call pthread_key_create is in the main thread where you wait
// after you wait for all other threads to terminate to ensure
// proper deallocation of resources.
void init_thread(void) {
  int errnum;

  printf("INIT_THREAD\n");

  if ((errnum = pthread_key_create(&key, free_keyed)) != 0) {
    handle_pthread_error(errnum);

    exit(EXIT_FAILURE);
  }
}

void *thread(void *arg) {
  int errnum;

  if ((errnum = pthread_once(&once_control, init_thread)) != 0) {
    handle_pthread_error(errnum);

    exit(EXIT_FAILURE);
  }

  create_thread_data();

  thread_data_t *tdata = pthread_getspecific(key);

  for (int i = 0; i < tdata->msg_reps; i++) {
    printf("TID: %lu, REPS: %d, I: %d, MSG: %s\n", tdata->tid, tdata->msg_reps,
           i, tdata->message);
  }

  return 0;
}

// we can essentially add a cleanup function which when the
// thread receives a cancel signal it will actually call the
// pushed thread function to do clean up before cancelling.

int main(void) {
  srand(time(NULL));

  int errnum;

  int num_of_threads = (rand() % 8) + 1;

  printf("NUMBER OF THREADS: %d\n", num_of_threads);

  pthread_t tids[num_of_threads];

  // thread_data_t tdatas[num_of_threads];

  for (int i = 0; i < num_of_threads; i++) {
    // snprintf(tdatas[i].message, MSG_LEN, "Thread: %d", i);

    if ((errnum = pthread_create(&tids[i], NULL, thread,
                                 NULL /*&tdatas[i]*/)) != 0) {
      handle_pthread_error(errnum);

      exit(EXIT_FAILURE);
    }
  }

  // unsigned int random_thread = rand() % num_of_threads;
  //
  // if ((errnum = pthread_cancel(tids[random_thread])) != 0) {
  //   // we need to make sure to handle this case because the
  //   // thread might not actually be alive anymore especially if
  //   // we detach.
  //   handle_pthread_error(errnum);
  // }

  void *return_value;

  for (int i = 0; i < num_of_threads; i++) {
    if ((errnum = pthread_join(tids[i], &return_value)) != 0) {
      handle_pthread_error(errnum);

      exit(EXIT_FAILURE);
    }

    if (return_value == PTHREAD_CANCELED) {
      printf("pthread_join: thread [%d] had been canceled\n", i);
    }
  }

  if ((errnum = pthread_key_delete(key)) != 0) {
    handle_pthread_error(errnum);
  }

  printf("DELETED_KEY\n");

  return 0;

  // he walks this path beware our saviour valgrind fails to
  // comprehend the magic which is threads and hence it will
  // always flag at num_of_threads as ... Actually don't even
  // bother I figured out it was an issue with and if condition.
}
