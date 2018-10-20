#include "vectorReduce.h"
#include <limits.h>

int result_parallel;

void *parallel_vector_reduce(void *args) {
  argument targs = *(argument *)args;

  int *a = targs.a;
  int accu = 0;

  int chunk_size = targs.size / targs.num_threads;
  int si = chunk_size * targs.tid;
  int ei;
  if (targs.tid == targs.num_threads - 1)
    ei = targs.size;
  else
    ei = si + chunk_size;

  while (si < ei)
    accu += a[si++];

  targs.ret_val = accu;
  pthread_exit(NULL);
}

int sequential_naive_vectorReduce(int *a, int size) {
  int accu = 0;
  int i;
  for (i = 0; i < size; i++)
    accu += a[i];
  return accu;
}

int main(int argc, char *argv[]) {
  int retstatus = EXIT_SUCCESS;
  struct timeval start, end;
  int *a;
  pthread_t *threads;
  argument *arguments;
  int result_serial;
  time_t t;

  if (argc != 3) {
    printf("Correct way to execute this program is:\n");
    printf("./vectorReduce arraySize NumberOfThreads.\n");
    return 1;
  }

  int size = atoi(argv[1]);
  int num_threads = atoi(argv[2]);

  a = (int *)malloc(sizeof(int) * size);
  threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
  arguments = (argument *)malloc(sizeof(argument) * num_threads);

  srand((unsigned)time(&t));

  result_serial = 0;
  result_parallel = 0;
  for (int i = 0; i < size; i++) {
    a[i] = rand() % 10;
  }

  gettimeofday(&start, NULL);

  result_serial = sequential_naive_vectorReduce(a, size);

  gettimeofday(&end, NULL);

  double diff =
      (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);

  printf("Naive VectorReduce time calculation duration: %.4fms\n", diff / 1000);

  gettimeofday(&start, NULL);

  for (int i = 0; i < num_threads; i++) {
    arguments[i].a = a;
    arguments[i].tid = i;
    arguments[i].size = size;
    /* arguments[i].ret_val; */
    arguments[i].num_threads = num_threads;

    pthread_create(&threads[i], NULL, parallel_vector_reduce,
                   (void *)&arguments[i]);
  }

  int accu = 0;
  for (int i = 0; i < num_threads; ++i) {
    pthread_join(threads[i], NULL);
    accu += arguments[i].ret_val;
  }
  result_parallel = accu;

  gettimeofday(&end, NULL);

  diff =
      (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);
  printf("Parallel VectorReduce time calculation duration: %.4fms\n",
         diff / 1000);

#ifdef TEST
  validate(&result_serial, &result_parallel, 1);
  printf("Reduction result = %d\n", result_serial);
#endif

exit_free:
  free(a);
  free(threads);
  free(arguments);

  return retstatus;
}
