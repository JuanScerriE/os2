/*
 * We can reference structs and other forms of data by actually
 * passing a pointer to the data and then cast the data
 * appropriately in the thread.
 *
 * We have no guarantee of how our threads are actually
 * interwieved and run on the host machine.
 *
 * The process exit function will generally terminate the whole
 * thread.
 *
 * The best way to do this is to acutally call pthread_exit to
 * kill the thread.
 *
 * Return from the start routine of the thread or they can
 * actually be cancelled.
 *
 *
 * Heisenbug; this is a special type of bug which appears when
 * we might appear when we actually debug our concurrent bug.
 * That is a concurrent bug actually disappears when we
 * instrument our code.
 *
 * We can cancel a thread will actively using pthread_cancel. It
 * might return an error.
 *
 * Suppose we might need a guarantee that a particular thread
 * actually needs to stop. If you do have a thread which waits
 * on some IO pthread_cancel can be very useful to actually
 * cancel that thread.
 *
 * We can also choose to manually set a thread to ignore
 * possibilities of being cancelled using
 * pthread_setcancelstate.
 *
 * We can also execute code before we actually cancel a thread.
 * pthread_cleanup_push and pthread_cleanup_pop
 *
 * Call the clean up function if int > 0;
 *
 * You can have more than cleanup handler. This is useful to
 * make sure that state is preserved.
 *
 * The thread control block is still retained if pthread_join is
 * not called. But if you cannot call pthread join we call
 * pthread_detach.
 *
 * We can specify scope, i.e. whether we have
 * pthread_scope_system and pthread_scope_process.
 * This specifies a threading preference.
 *
 * Set whether thread competes for resources with all
 * other threads within the same process
 * (PTHREAD_SCOPE_PROCESS); or competes with all other
 * threads in all other processes on the system
 * (PTHREAD_SCOPE_SYSTEM)
 *
 * pthread attributes
 *  - stach address and size of the the thread
 *  - stack size
 *  - setschedparam > set scheduling priority
 *
 * • detachstate:
 *	– PTHREAD_CREATE_JOINABLE (default)
 *		• Threads created in a joinable state; assumed that creating
 *		thread will be waiting for it
 *	– PTHREAD_CREATE_DETACHED
 *		• Threads created in a detached state; its thread ID and
 *		resources can be reused as soon as thread terminates;
 *		use this when the calling thread will not want to wait
 *		for created thread
 *
 *
 *
 * Thread Specific Data
 *
 * We can initialise some data associated with that thread using
 * pthread_key_t key. Ideally, each thread uses its own
 * particular data.
 *
 * Then are we going to delete that thread specific data.
 * You can set it initialise once. i.e. use pthread_one_t;
 * Hence, you can let a thread use the same that once accross
 * the whole program
 *
 * e.g. we make sure that the thread does not actually fail
 * because it might be the case that a thread fails. (e.g. of
 * synchronoisation problems Therac-25)
 *
 * Then we join on each thread. We can 
 * specify a piece of code which only runs once only when the
 * first request comes in. This where we can use pthread_once.
 *
 * Next lecture is going to be about synchronizations
 *
 * and we will do the execrises in the pthread functionality
 * which we have.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct thread_data {
  long threadid;
  char *message;
} thread_data_t;

void *thread(void *data) {
  thread_data_t *casted_data = (thread_data_t *)data;
  printf("From inside: { .threadid = %ld, .message = \"%s\" }\n",
         casted_data->threadid, casted_data->message);
  pthread_exit(NULL);
}

// void *MyThread(void *arg) { thread_data_t *data; }

int main(void) {
  int returnval;
  pthread_t tid;

  thread_data_t *data = (thread_data_t *)malloc(sizeof(thread_data_t));
  data->threadid = 10;
  data->message = "Hello, Amy";

  returnval = pthread_create(&tid, NULL, thread, data);

  printf("returnval: %d\n", returnval);

  int test = 0;
  while (test++ < 100000) {
  }

  printf("Pthreads example!\n");

  pthread_join(tid, NULL);

  return 0;
}

void *MyThread(void *arg) {
  thread_data_t *data;
  int i;
  data = (thread_data_t *)arg;
  for (i = 0; i < 10; i++) {
    printf("From inside the created thread: tid [%ld] message [%s]\n",
           data->threadid, data->message);
    sleep(1);
  }
  pthread_exit(NULL);
}
