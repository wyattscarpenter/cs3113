#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

static unsigned int global = 0;
static sem_t sem;

static void *                   /* Loop 'arg' times incrementing 'global' */
threadFunc(void *arg)
{
    int loops = *((int *) arg);
    int loc, j;

    for (j = 0; j < loops; j++) {
        if (sem_wait(&sem) == -1)
	  exit(-1);

        global++;

        if (sem_post(&sem) == -1)
	  exit(-1);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
  int MAX_THREADS = 255;
    pthread_t t[MAX_THREADS];
    long loops, threads, s;

    loops = atol(argv[1]);
    threads = atol(argv[2]);
    
    /* Initialize a semaphore with the value 1 */

    if (sem_init(&sem, 0, 1) == -1)
      exit(-1);

    /* Create n threads that increment 'global' */
    for(int i = 0; i < threads; i++){
      s = pthread_create(&t[i], NULL, threadFunc, &loops);
      if (s != 0)
	exit(-1);
    }
      /* Wait for threads to terminate */
    for(int i = 0; i < threads; i++){
    s = pthread_join(t[i], NULL);
    if (s != 0)
      exit(-1);
    }
    printf("global = %d\n", global);
    exit(EXIT_SUCCESS);
}
