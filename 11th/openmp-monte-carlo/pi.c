#ifndef _HISTOGRAM_H
#define _HISTOGRAM_H
#include "pi.h"
#endif

int main(int argc, char *argv[]) {

  double elapsed_time;
  double serialPI = 0, parallelPI = 0;

  if (argc != 3) {
    printf("Correct way to execute this program is:\n");
    printf("./pi stepNum numberOfThreads\n");
    return 1;
  }

  int num_steps = atoi(argv[1]);
  int num_thread = atoi(argv[2]);

  // Sequential histogram
  set_clock();

  // COMPLETE HERE
  time_t t;
  srand((unsigned)time(&t));

  double x = 0, y = 0;
  double sum = 0;
  int count_inside = 0;
  unsigned int seed = 11;
  for (int i = 0; i < num_steps; i++) {
    x = (double)rand_r(&seed) / RAND_MAX;
    y = (double)rand_r(&seed) / RAND_MAX;
    sum = x * x + y * y;
    if (sum <= 1) {
      count_inside++;
    }
  }
  serialPI = (count_inside * 4.0) / num_steps;

  elapsed_time = get_elapsed_time();

  printf("Naive PI calculation time: %.4fms\n", elapsed_time / 1000);

  // Openmp Parallel histogram
  set_clock();

  omp_set_num_threads(num_thread);

#pragma omp parallel
  {
    double x, y;
    int count_inside_shared = 0;
    unsigned int seed = 11 * omp_get_thread_num();
#pragma omp for
    for (int i = 0; i < num_steps; i++) {
      x = (double)rand_r(&seed) / RAND_MAX;
      y = (double)rand_r(&seed) / RAND_MAX;
      if (x * x + y * y <= 1.0)
        count_inside_shared++;
    }
#pragma omp atomic
    parallelPI += count_inside_shared;
  }
  parallelPI = (parallelPI * 4.0) / num_steps;

  elapsed_time = get_elapsed_time();

  printf("-> Openmp PI calculation time: %.4fms\n", elapsed_time / 1000);

#ifdef TEST
  validate(&serialPI, &parallelPI, 1);
#endif

  return 0;
}
