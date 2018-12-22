#include "vectorOp.h"

__global__ void
vector_operation_kernel(int* output, int* data, int size)
{
  for (uint tid = blockDim.x * blockIdx.x + threadIdx.x; tid < size;
       tid += blockIdx.x * gridDim.x)
    output[tid] = OPERATION_I(data[tid]);
}

void
sequential_vector_operation(int* output, int* data, int size)
{
  for (int i = 0; i < size; i++) {
    output[i] = OPERATION_I(data[i]);
  }
  return;
}

int
main(int argc, char* argv[])
{
  double elapsed_time_CUDA, elapsed_time_CPU;
  int block_size, grid_size;
  int data_size;
  int *data_h, *output_h, *device_output_h;
  int *data_d, *output_d;
  int work_per_thrd;

  if (argc != 4) {
    printf("Correct way to execute this program is:\n");
    printf("./vectorOp data_size block_size work_per_thread\n");
    printf("For example:\n./vectorOp 10000 512 4\n");
    return 1;
  }

  data_size = atoi(argv[1]);
  block_size = atoi(argv[2]);
  work_per_thrd = atoi(argv[3]);

  // Initialize data on Host
  initialize_data_random(&data_h, data_size);
  initialize_data_zero(&output_h, data_size);
  initialize_data_zero(&device_output_h, data_size);

  // Initialize data on Device
  CUDA_CHECK_RETURN(cudaMalloc((void**)&data_d, sizeof(int) * data_size));
  CUDA_CHECK_RETURN(cudaMalloc((void**)&output_d, sizeof(int) * data_size));

  // Sequential vector operation
  set_clock();
  {
    sequential_vector_operation(output_h, data_h, data_size);
  }
  elapsed_time_CPU = get_elapsed_time();
  printf("-> Naive vector operation time: %.4fms\n", elapsed_time_CPU / 1000);

  // CUDA Parallel vector operation
  grid_size = (data_size - 1) / block_size * work_per_thrd + 1; // FIXME
  dim3 grid_dime(grid_size, 1, 1);
  dim3 block_dime(block_size, 1, 1);

  set_clock();
  {
    CUDA_CHECK_RETURN(cudaMemcpy(
      data_d, data_h, sizeof(int) * data_size, cudaMemcpyHostToDevice));

    // vector_operation_kernel<<<grid_dime, block_dime>>>(
    // output_d, data_d, work_per_thrd, data_size);

    vector_operation_kernel<<<grid_dime, block_dime>>>(
      output_d, data_d, data_size);

    // Wait for the GPU launched work to complete
    CUDA_CHECK_RETURN(cudaDeviceSynchronize());
    CUDA_CHECK_RETURN(cudaGetLastError());

    // Copy back the result
    CUDA_CHECK_RETURN(cudaMemcpy(device_output_h,
                                 output_d,
                                 sizeof(int) * data_size,
                                 cudaMemcpyDeviceToHost));
  }
  elapsed_time_CUDA = get_elapsed_time();

  printf("-> CUDA vector operation time: %.4fms\n", elapsed_time_CPU / 1000);
  printf("Achived speedup: %.4f\n", elapsed_time_CPU / elapsed_time_CUDA);

#ifdef TEST
  validate(output_h, device_output_h, data_size);
#endif

  free(data_h);
  free(output_h);
  free(device_output_h);

  CUDA_CHECK_RETURN(cudaFree(output_d));
  CUDA_CHECK_RETURN(cudaFree(data_d));

  return 0;
}
