#include <stdio.h>
#include <string.h>
#include "../source/cl-utils.h"
#include "../utils/timing.h"
#include "../utils/utils.h"

#define DMA 0
#define NUM_INT 100
#define NUM_RUNS 100

void calculate_dimensions(cl_device_id *, size_t[3], size_t[3], int, int);

int main(int argc, const char **argv) {

  // create queue and context
  cl_context ctx;
  cl_command_queue queue_inputs;
  cl_command_queue queue_kernel;
  cl_command_queue queue_results;
  cl_int err;
  cl_device_id device;
  create_context_on_gpu(&ctx, &device, 0);
  create_command_queue(&queue_inputs, &ctx, &device);
  create_command_queue(&queue_kernel, &ctx, &device);
  create_command_queue(&queue_results, &ctx, &device);

  struct timespec ete_start, ete_end;
  cl_ulong ev_start_time, ev_end_time;
  size_t goffset[3] = {0, 0, 0};
  size_t gdim[3], ldim[3]; // assuming three dimensions
  int ldim0 = 0;
  calculate_dimensions(&device, gdim, ldim, NUM_INT, ldim0);

  // create kernel
  char *knl_text = read_file("kernel.cl");
  if (!knl_text) {
    printf("Couldn't read file kernel.c. Exiting!\n");
    return -1;
  }
  cl_kernel knl;
  knl = kernel_from_string(ctx, knl_text, "kernel_fun", "");
  free(knl_text);

  // declare arrays
  int num_inputs = NUM_INT;
  size_t size_inputs_total = sizeof(int) * NUM_INT;
  int *inputs = (int *)malloc(size_inputs_total);
  int *results = (int *)malloc(size_inputs_total);

  // populate array
  for (int i = 0; i < NUM_INT; i++) {
    inputs[i] = i;
  }

#if DMA
  printf("DMA!\n");
#else
  printf("NOT DMA!\n");
#endif

  printf("trans-inputs\ttrans-results\texec-kernel\ttime-total\n");

  for (int j = 0; j < NUM_RUNS; j++) {

    // create buffers
    cl_mem buf_inputs =
        clCreateBuffer(ctx, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE,
                       size_inputs_total, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer buf_inputs: %d\n", err);

    cl_mem buf_results =
        clCreateBuffer(ctx, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE,
                       size_inputs_total, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer buf_results: %d\n", err);

    // set kernel arguments
    err = clSetKernelArg(knl, 0, sizeof(cl_mem), &buf_inputs);
    if (err != CL_SUCCESS)
      printf("error: clSetKernelArg 0: %d\n", err);

    err = clSetKernelArg(knl, 1, sizeof(cl_mem), &buf_results);
    if (err != CL_SUCCESS)
      printf("error: clSetKernelArg 1: %d\n", err);

    err = clSetKernelArg(knl, 2, sizeof(int), &num_inputs);
    if (err != CL_SUCCESS)
      printf("error: clSetKernelArg 2: %d\n", err);

    // flush queues before timing
    err = clFinish(queue_inputs);
    if (err != CL_SUCCESS)
      printf("error: clFinish queue_inputs: %d\n", err);

    err = clFinish(queue_kernel);
    if (err != CL_SUCCESS)
      printf("error: clFinish queue_kernel: %d\n", err);

    err = clFinish(queue_results);
    if (err != CL_SUCCESS)
      printf("error: clFinish queue_results: %d\n", err);

    // declare events
    cl_event event_inputs;
    cl_event event_results;
    cl_event event_kernel;

#if DMA

    struct timespec ete_start_inputs, ete_end_inputs;
    struct timespec ete_start_results, ete_end_results;

    get_timestamp(&ete_start);

    // declare/map inputs array
    get_timestamp(&ete_start_inputs);

    int *inputs_dma =
        clEnqueueMapBuffer(queue_inputs, buf_inputs, CL_TRUE, CL_MAP_WRITE, 0,
                           size_inputs_total, 0, NULL, &event_inputs, &err);
    if (err != CL_SUCCESS)
      printf("error: clEnqueueMapBuffer inputs: %d\n", err);

    // populate inputs array
    memcpy(inputs_dma, inputs, size_inputs_total);

    // unmap inputs array
    err = clEnqueueUnmapMemObject(queue_inputs, buf_inputs, inputs_dma, 0, NULL,
                                  NULL);
    if (err != CL_SUCCESS)
      printf("error: clEnqueueUnmapMemObject inputs: %d\n", err);

    get_timestamp(&ete_end_inputs);

    // launch kernel
    err = clEnqueueNDRangeKernel(queue_kernel, knl, 1, goffset, gdim, ldim, 1,
                                 &event_inputs, &event_kernel);
    if (err != CL_SUCCESS)
      printf("error: clEnqueueNDRangeKernel: %d\n", err);

    // declare/map results array
    get_timestamp(&ete_start_results);

    int *results_dma = clEnqueueMapBuffer(queue_results, buf_results, CL_FALSE,
                                          CL_MAP_READ, 0, size_inputs_total, 1,
                                          &event_kernel, &event_results, &err);
    if (err != CL_SUCCESS)
      printf("error: clEnqueueMapBuffer results: %d\n", err);

    // populate results array
    memcpy(results, results_dma, size_inputs_total);

    // unmap results array
    err = clEnqueueUnmapMemObject(queue_results, buf_results, results_dma, 0,
                                  NULL, NULL);
    if (err != CL_SUCCESS)
      printf("error: clEnqueueUnmapMemObject results: %d\n", err);

    get_timestamp(&ete_end_results);

#else

    get_timestamp(&ete_start);

    err =
        clEnqueueWriteBuffer(queue_inputs, buf_inputs, CL_FALSE, 0,
                             size_inputs_total, inputs, 0, NULL, &event_inputs);
    if (err != CL_SUCCESS)
      printf("error: clEnqueueWriteBuffer: %d\n", err);

    // launch kernel
    err = clEnqueueNDRangeKernel(queue_kernel, knl, 1, goffset, gdim, ldim, 1,
                                 &event_inputs, &event_kernel);
    if (err != CL_SUCCESS)
      printf("error: clEnqueueNDRangeKernel: %d\n", err);

    // transfer results
    err = clEnqueueReadBuffer(queue_results, buf_results, CL_FALSE, 0,
                              size_inputs_total, results, 0, NULL,
                              &event_results);
    if (err != CL_SUCCESS)
      printf("error: clEnqueueWriteBuffer: %d\n", err);
#endif

    // finish the queues
    err = clFinish(queue_inputs);
    if (err != CL_SUCCESS)
      printf("error: clFinish queue_inputs: %d\n", err);

    err = clFinish(queue_kernel);
    if (err != CL_SUCCESS)
      printf("error: clFinish queue_kernel: %d\n", err);

    err = clFinish(queue_results);
    if (err != CL_SUCCESS)
      printf("error: clFinish queue_results: %d\n", err);

    get_timestamp(&ete_end);

    // free memory buffers
    err = clReleaseMemObject(buf_inputs);
    if (err != CL_SUCCESS)
      printf("error: clReleaseMemObject: %d\n", err);

    err = clReleaseMemObject(buf_results);
    if (err != CL_SUCCESS)
      printf("error: clReleaseMemObjec: %d\n", err);

    // get timing
    double trans_inputs = 0.0;
    double trans_results = 0.0;
    double time_gpu = 0.0;
    double end_to_end = 0.0;

#if DMA

    trans_inputs = timestamp_diff_in_seconds(ete_start_inputs, ete_end_inputs) *
                   1000; // in ms
    trans_results =
        timestamp_diff_in_seconds(ete_start_results, ete_end_results) *
        1000; // in ms

#else
    clGetEventProfilingInfo(event_inputs, CL_PROFILING_COMMAND_START,
                            sizeof(cl_ulong), &ev_start_time, NULL);
    clGetEventProfilingInfo(event_inputs, CL_PROFILING_COMMAND_END,
                            sizeof(cl_ulong), &ev_end_time, NULL);
    trans_inputs = (double)(ev_end_time - ev_start_time) / 1000000;

    clGetEventProfilingInfo(event_results, CL_PROFILING_COMMAND_START,
                            sizeof(cl_ulong), &ev_start_time, NULL);
    clGetEventProfilingInfo(event_results, CL_PROFILING_COMMAND_END,
                            sizeof(cl_ulong), &ev_end_time, NULL);
    trans_results = (double)(ev_end_time - ev_start_time) / 1000000;
#endif

    clGetEventProfilingInfo(event_kernel, CL_PROFILING_COMMAND_START,
                            sizeof(cl_ulong), &ev_start_time, NULL);
    clGetEventProfilingInfo(event_kernel, CL_PROFILING_COMMAND_END,
                            sizeof(cl_ulong), &ev_end_time, NULL);
    time_gpu = (double)(ev_end_time - ev_start_time) / 1000000;

    end_to_end = timestamp_diff_in_seconds(ete_start, ete_end) * 1000; // in ms
    printf("%.6f\t%.6f\t%.6f\t%.6f\n", trans_inputs, trans_results, time_gpu,
           end_to_end);
  }
  // print results
  for (int i = 0; i < NUM_INT; i++) {
    printf("%d ", results[i]);
  }
  printf("\n");

  return 0;
}

void calculate_dimensions(cl_device_id *device, size_t gdim[3], size_t ldim[3],
                          int num_test_cases, int ldimsupplied) {
  // find out maximum dimensions for device
  cl_int err;

  cl_uint num_dims;
  err = clGetDeviceInfo(*device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                        sizeof(num_dims), &num_dims, NULL);
  if (err != CL_SUCCESS)
    printf("error: clGetDeviceInfo CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: %d\n",
           err);

  size_t dims[num_dims];
  err = clGetDeviceInfo(*device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(dims),
                        dims, NULL);
  if (err != CL_SUCCESS)
    printf("error: clGetDeviceInfo CL_DEVICE_MAX_WORK_ITEM_SIZES: %d\n", err);

  // calculate local dimension
  int ldim0 = num_test_cases;

  if (ldimsupplied != 0) {
    // use the given dimension
    ldim0 = ldimsupplied;
  } else {
    // calculate a dimension
    int div = num_test_cases / dims[0]; // maximum size per work-group
    if (div > 0)
      ldim0 = num_test_cases / (div + 1);

    // ensure that the dimensions will be properly distributed across
    while ((num_test_cases / ldim0) * ldim0 != num_test_cases) {
      div++;
      if (div > 0)
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
