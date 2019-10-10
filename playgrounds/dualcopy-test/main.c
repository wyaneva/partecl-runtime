#include <stdio.h>
#include <string.h>
#include "../../source/cl-utils.h"
#include "../../utils/timing.h"
#include "../../utils/utils.h"

#define NUM_INT 1024*10000
//#define NUM_INT 10
#define NUM_RUNS 20 
//#define NUM_RUNS 1
#define NUM_CHUNKS 8

void calculate_dimensions(cl_device_id *, size_t[3], size_t[3], int, int);

int main(int argc, const char **argv) {

  cl_int err;

  // create the context
  cl_context ctx;
  cl_device_id device;
  create_context_on_gpu(&ctx, &device, 0);

  // create two command queues
  cl_command_queue queue[NUM_CHUNKS];
  for(int i = 0; i < NUM_CHUNKS; i++) {
    create_command_queue(&queue[i], &ctx, &device);
  }

    // create kernels - one per command queue
  char *knl_text = read_file("kernel.cl");
  if (!knl_text) {
    printf("Couldn't read file kernel.c. Exiting!\n");
    return -1;
  }
  cl_kernel knl[NUM_CHUNKS];
  for(int i = 0; i < NUM_CHUNKS; i++) {
    knl[i] = kernel_from_string(ctx, knl_text, "kernel_fun", "");
  }
  free(knl_text);

  // calculate memory
  int num_inputs = NUM_INT;
  int num_inputs_chunk = num_inputs / NUM_CHUNKS;
  size_t size_inputs_total = sizeof(int) * num_inputs;
  size_t size_inputs_chunk = size_inputs_total / NUM_CHUNKS;

  // calculate workgroup offsets
  size_t goffset[3] = {0, 0, 0};
  size_t gdim[3], ldim[3]; // assuming three dimensions
  int ldim0 = 0;
  calculate_dimensions(&device, gdim, ldim, num_inputs_chunk, ldim0);

  // declare the OpenCL input and result buffer memory objects on GPU device
  cl_mem buf_dev_inputs;
  cl_mem buf_dev_results;

  // allocate the OpenCL input and result buffer memory objects on GPU device
  buf_dev_inputs = clCreateBuffer(ctx, CL_MEM_READ_ONLY, size_inputs_chunk, NULL, &err);
  if (err != CL_SUCCESS) printf("error: clCreateBuffer buf_dev_inputs 0: %d\n", err);
  buf_dev_results = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, size_inputs_total, NULL, &err);
  if (err != CL_SUCCESS) printf("error: clCreateBuffer buf_dev_results 0: %d\n", err);

  // set kernel arguments
  for(int i = 0; i < NUM_CHUNKS; i++) {
    err = clSetKernelArg(knl[i], 0, sizeof(cl_mem), &buf_dev_inputs);
    if (err != CL_SUCCESS) printf("error: clSetKernelArg 0: %d\n", err);
    err = clSetKernelArg(knl[i], 1, sizeof(cl_mem), &buf_dev_results);
    if (err != CL_SUCCESS) printf("error: clSetKernelArg 1: %d\n", err);
    err = clSetKernelArg(knl[i], 2, sizeof(int), &num_inputs_chunk);
    if (err != CL_SUCCESS) printf("error: clSetKernelArg 2: %d\n", err);
  }

  // declare pinned input and result host buffers
  cl_mem pinned_host_inputs[NUM_CHUNKS]; 
  cl_mem pinned_host_results[NUM_CHUNKS]; 
  // allocate pinned input and result host buffers
  for(int i = 0; i < NUM_CHUNKS; i++) {
    pinned_host_inputs[i] = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, size_inputs_chunk, NULL, &err);
    if (err != CL_SUCCESS) printf("error: clCreateBuffer pinned_host_inputs %d: %d\n", i, err);
    pinned_host_results[i] = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, size_inputs_chunk, NULL, &err);
    if (err != CL_SUCCESS) printf("error: clCreateBuffer pinned_host_results %d: %d\n", i, err);
  }

  // mapped pointers to pinned host buffers (to access them using standard pointers)
  int *inputs[NUM_CHUNKS];
  int *results[NUM_CHUNKS];
  // mapped pointers to pinned host buffers (to access them using standard pointers)
  for(int i = 0; i < NUM_CHUNKS; i++) {
    inputs[i] = (cl_int*)clEnqueueMapBuffer(queue[0], pinned_host_inputs[i], CL_TRUE, CL_MAP_WRITE, 0, size_inputs_chunk, 0, NULL, NULL, &err);
    if (err != CL_SUCCESS) printf("error: clEnqueueMapBuffer pinned_host_inputs: %d\n", err);
    results[i] = (cl_int*)clEnqueueMapBuffer(queue[0], pinned_host_results[i], CL_TRUE, CL_MAP_READ, 0, size_inputs_chunk, 0, NULL, NULL, &err);
    if (err != CL_SUCCESS) printf("error: clEnqueueMapBuffer pinned_host_results: %d\n", err);
  }

  // populate inputs
  for(int i = 0; i < NUM_CHUNKS; i++) {
    for (int k = 0; k < num_inputs_chunk; k++) {
      inputs[i][k] = i*num_inputs_chunk + k;
    }
  }

  // flush queues before timing
  for(int i = 0; i < NUM_CHUNKS; i++) {
    err = clFinish(queue[i]);
    if (err != CL_SUCCESS) printf("error: clFinish queue[0]: %d\n", err);
  }

  printf("trans-inputs\ttrans-results\texec-kernel\trate-inputs\trate-results\ttime-total\n");
  //printf("trans-inputs\ttrans-results\texec-kernel\ttime-total\n");
  for (int j = 0; j < NUM_RUNS; j++) {

    // declare events
    cl_event event_inputs[NUM_CHUNKS];
    cl_event event_results[NUM_CHUNKS];
    cl_event event_kernel[NUM_CHUNKS];

    struct timespec ete_start, ete_end;
    get_timestamp(&ete_start);

    for(int i = 0; i < NUM_CHUNKS; i++) {

      // Nonblocking write of chunk of input data from host to device 
      err = clEnqueueWriteBuffer(queue[i], buf_dev_inputs, CL_FALSE, 0, size_inputs_chunk, (void*)&inputs[i][0], 0, NULL, &event_inputs[i]);
      if (err != CL_SUCCESS) printf("error: clEnqueueWriteBuffer buf_dev_inputs %d: %d\n", i, err);

      // Launch kernel computation 
      err = clEnqueueNDRangeKernel(queue[i], knl[i], 1, goffset, gdim, ldim, 1, &event_inputs[i], &event_kernel[i]);
      if (err != CL_SUCCESS) printf("error: clEnqueueNDRangeKernel queue %d: %d\n", i, err);

      // Nonblocking read of chunk of results from device to host 
      err = clEnqueueReadBuffer(queue[i], buf_dev_results, CL_FALSE, 0, size_inputs_chunk, (void*)&results[i][0], 1, &event_kernel[i], &event_results[i]);
      if (err != CL_SUCCESS) printf("error: clEnqueueReadBuffer buf_dev_results %d: %d\n", i, err);

    }

    // Finish the queues
    for(int i = 0; i < NUM_CHUNKS; i++) {
      err = clFinish(queue[i]);
      if (err != CL_SUCCESS) printf("error: clFinish queue[%d]: %d\n", i, err);
    }

    get_timestamp(&ete_end);

    // Print timing
    double time_inputs = 0.0;
    double time_results = 0.0;
    double time_kernel = 0.0;
    double end_to_end = 0.0;
    double transfer_rate_inputs = 0.0;
    double transfer_rate_results = 0.0;
    
    end_to_end = timestamp_diff_in_seconds(ete_start, ete_end) * 1000; // in ms

    cl_ulong ev_start_time, ev_end_time;
    for(int i = 0; i < NUM_CHUNKS; i++) {

      clGetEventProfilingInfo(event_inputs[i], CL_PROFILING_COMMAND_START,
                            sizeof(cl_ulong), &ev_start_time, NULL);
      clGetEventProfilingInfo(event_inputs[i], CL_PROFILING_COMMAND_END,
                            sizeof(cl_ulong), &ev_end_time, NULL);
      time_inputs = (double)(ev_end_time - ev_start_time) / 1000000;

      clGetEventProfilingInfo(event_results[i], CL_PROFILING_COMMAND_START,
                            sizeof(cl_ulong), &ev_start_time, NULL);
      clGetEventProfilingInfo(event_results[i], CL_PROFILING_COMMAND_END,
                            sizeof(cl_ulong), &ev_end_time, NULL);
      time_results = (double)(ev_end_time - ev_start_time) / 1000000;

      clGetEventProfilingInfo(event_kernel[i], CL_PROFILING_COMMAND_START,
                            sizeof(cl_ulong), &ev_start_time, NULL);
      clGetEventProfilingInfo(event_kernel[i], CL_PROFILING_COMMAND_END,
                            sizeof(cl_ulong), &ev_end_time, NULL);
      time_kernel = (double)(ev_end_time - ev_start_time) / 1000000;

      transfer_rate_inputs = (size_inputs_chunk * 0.001 * 0.001) / time_inputs; // in GB/s
      transfer_rate_results = (size_inputs_chunk * 0.001 * 0.001) / time_results; // in GB/s

      if(i == NUM_CHUNKS-1) {
        printf("%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\n", time_inputs, time_results, time_kernel, 
            transfer_rate_inputs, transfer_rate_results, end_to_end);
        //printf("%.6f\t%.6f\t%.6f\t%.6f\n", time_inputs, time_results, time_kernel, end_to_end);
      } else {
        printf("%.6f\t%.6f\t%.6f\t%.6f\t%.6f\n", time_inputs, time_results, time_kernel,
            transfer_rate_inputs, transfer_rate_results);
        //printf("%.6f\t%.6f\t%.6f\n", time_inputs, time_results, time_kernel);
      }
    }
  }

  //TODO Check results
  /*
  for(int i = 0; i < NUM_CHUNKS; i++) {
    for(int j = 0; j < num_inputs_chunk; j++) {
      printf("%d %d: %d %d\n", i, j, inputs[i][j], results[i][j]);
    }
  }
  */

  // Cleanup

  // Free device memory buffers
  err = clReleaseMemObject(buf_dev_inputs);
  if (err != CL_SUCCESS) printf("error: clReleaseMemObject buf_dev_inputs: %d\n", err);
  err = clReleaseMemObject(buf_dev_results);
  if (err != CL_SUCCESS) printf("error: clReleaseMemObjec buf_dev_results: %d\n", err);

  for(int i = 0; i < NUM_CHUNKS; i++) {
    // Unmap pinned memory
    err = clEnqueueUnmapMemObject(queue[0], pinned_host_inputs[i], (void*)inputs[i], 0, NULL, NULL);
    if (err != CL_SUCCESS) printf("error: clEnqueueUnmapMemObject pinned_host_inputs %d: %d\n", i, err);
    err = clEnqueueUnmapMemObject(queue[0], pinned_host_results[i], (void*)results[i], 0, NULL, NULL);
    if (err != CL_SUCCESS) printf("error: clEnqueueUnmapMemObject pinned_host_results %d: %d\n", i, err);

    // Free pinned host memory buffers
    err = clReleaseMemObject(pinned_host_inputs[i]);
    if (err != CL_SUCCESS) printf("error: clReleaseMemObject pinned_host_inputs %d: %d\n", i, err);
    err = clReleaseMemObject(pinned_host_results[i]);
    if (err != CL_SUCCESS) printf("error: clReleaseMemObject pinned_host_results %d: %d\n", i, err);

    // Release command queues and context
    err = clReleaseKernel(knl[i]);
    err = clReleaseCommandQueue(queue[i]);
  }

  err = clReleaseContext(ctx);
  free(device);
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
