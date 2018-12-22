#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define MIN(a, b) ((a) < (b) ? a : b)

typedef struct {
  int *a;
  int tid;
  int size;
  int ret_val;
  int num_threads;
} argument;

int validate(int *a, int *b, int length) {
  for (int i = 0; i < length; ++i) {
    if (a[i] != b[i]) {
      printf("Different value detected at position: %d,"
	     "expected %d but get %d\n",
	     i, a[i], b[i]);
      return 1;
    }
  }
  printf("Validation test passed successfully.\n");
  return 0;
}
