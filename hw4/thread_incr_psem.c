#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

static int global = 0;
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
    pthread_t t1, t2;
    int loops, threads, s;

    loops = atol(argv[1]);

    /* Initialize a semaphore with the value 1 */

    if (sem_init(&sem, 0, 1) == -1)
      exit(-1);

    /* Create two threads that increment 'global' */

    s = pthread_create(&t1, NULL, threadFunc, &loops);
    if (s != 0)
      exit(-1);
    s = pthread_create(&t2, NULL, threadFunc, &loops);
    if (s != 0)
      exit(-1);
      /* Wait for threads to terminate */

    s = pthread_join(t1, NULL);
    if (s != 0)
      exit(-1);
    s = pthread_join(t2, NULL);
    if (s != 0)
      exit(-1);
    printf("global = %d\n", global);
    exit(EXIT_SUCCESS);
}
