#include <stdio.h>
#include "../utils/options.h"
#include "../utils/timing.h"
#include "kernel/test.cu"

extern "C" {
#include "kernel.h"
}

void calculate_dimensions(size_t[3], size_t[3], int, int);

/*
__global__ void main_kernel(struct partecl_input* inputs, struct partecl_result* results)
{
  printf("Hello %d!\n", calculateGlobalThreadId());
}
*/

extern "C" void exec_kernel(
    int do_compare_results,
    int num_runs,
    int do_time,
    int ldim0,
    int num_chunks,
    int num_test_cases,
    struct partecl_input *inputs,
    struct partecl_result *results,
    size_t size_inputs,
    size_t size_results)
{
  int chunksize = num_test_cases/num_chunks;

  //clalculate dimensions
  size_t gdim[3], ldim[3]; //assuming three dimensions
  calculate_dimensions(/*&device, */gdim, ldim, chunksize, ldim0);
  printf("LDIM = %zd\n", ldim[0]);

  if(do_time)
  {
    printf("Number of test cases: %d\n", num_test_cases);
    printf("Time in ms\n");
    printf("trans-inputs trans-results exec-kernel time-total \n");
  }

  for(int i=0; i < num_runs; i++)
  {
    //timing variables
    float trans_inputs = 0.0;
    float trans_results = 0.0;
    float time_gpu = 0.0;
    double end_to_end = 0.0;
    struct timespec ete_start, ete_end;
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    //allocate device memory
    partecl_input *d_inputs;
    cudaMalloc(&d_inputs, size_inputs);

    partecl_result *d_results;
    cudaMalloc(&d_results, size_results);

    get_timestamp(&ete_start);

    for(int j = 0; j < num_chunks; j++)
    {
      //transfer input to device
      cudaEventRecord(start);
      cudaMemcpy(d_inputs, inputs, size_inputs, cudaMemcpyHostToDevice);
      cudaEventRecord(stop);
      cudaEventSynchronize(stop);
      cudaEventElapsedTime(&trans_inputs, start, stop);

      //launch kernel
      dim3 grid(gdim[0]/ldim[0], gdim[1]/ldim[1], gdim[2]/ldim[2]);
      dim3 block(ldim[0], ldim[1], ldim[2]);
      cudaEventRecord(start);
      main_kernel<<<grid, block>>>(d_inputs, d_results);
      cudaEventRecord(stop);
      cudaEventSynchronize(stop);
      cudaEventElapsedTime(&time_gpu, start, stop);

      //transfer results back
      cudaEventRecord(start);
      cudaMemcpy(results, d_results, size_results, cudaMemcpyDeviceToHost);
      cudaEventRecord(stop);
      cudaEventSynchronize(stop);
      cudaEventElapsedTime(&trans_results, start, stop);
   }

   get_timestamp(&ete_end);

   //free memory buffers
   cudaFree(d_inputs);
   cudaFree(d_results);

   end_to_end = timestamp_diff_in_seconds(ete_start, ete_end) * 1000; //in ms
   if(do_time)
    printf("%f %f %f %f \n", trans_inputs, trans_results, time_gpu, end_to_end);
  }
}

void calculate_dimensions(/*cl_device_id *device, */size_t gdim[3], size_t ldim[3], int num_test_cases, int ldimsupplied)
{
  //find out maximum dimensions for device
  struct cudaDeviceProp props;
  cudaGetDeviceProperties(&props, 0);
  size_t dims[3];
  dims[0] = props.maxThreadsDim[0];
  dims[1] = props.maxThreadsDim[1];
  dims[2] = props.maxThreadsDim[2];

  //calculate local dimension
  int ldim0 = num_test_cases;

  if(ldimsupplied != LDIM)
  {
    //use the given dimension
    ldim0 = ldimsupplied;
  }
  else
  {
    //calculate a dimension
    int div = num_test_cases/ dims[0]; //maximum size per work-group
    if(div > 0)
      ldim0 = num_test_cases/ (div+1);

    //ensure that the dimensions will be properly distributed across 
    while((num_test_cases / ldim0) * ldim0 != num_test_cases)
    {
      div++;
      if(div > 0)
        ldim0 = num_test_cases / div;
    }
  }

  gdim[0] = num_test_cases;
  gdim[1] = 1;
  gdim[2] = 1;
  ldim[0] = ldim0;
  ldim[1] = 1;
  ldim[2] = 1;
}

