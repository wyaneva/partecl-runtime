#include <stdio.h>
#include <string.h>
#include "../source/cl-utils.h"
#include "../utils/timing.h"
#include "../utils/utils.h"

#define NUM_INT 1024*10000
//#define NUM_INT 10
#define NUM_RUNS 50
//#define NUM_RUNS 1
#define NUM_CHUNKS 2

void calculate_dimensions(cl_device_id *, size_t[3], size_t[3], int, int);

int main(int argc, const char **argv) {

  cl_int err;

  // create the context
  cl_context ctx;
  cl_device_id device;
  create_context_on_gpu(&ctx, &device, 0);

  // create two command queues
  cl_command_queue queue[2];
  create_command_queue(&queue[0], &ctx, &device);
  create_command_queue(&queue[1], &ctx, &device);

    // create kernels - one per command queue
  char *knl_text = read_file("kernel.cl");
  if (!knl_text) {
    printf("Couldn't read file kernel.c. Exiting!\n");
    return -1;
  }
  cl_kernel knl[2];
  knl[0] = kernel_from_string(ctx, knl_text, "kernel_fun", "");
  knl[1] = kernel_from_string(ctx, knl_text, "kernel_fun", "");
  free(knl_text);

  // calculate memory
  int num_inputs = NUM_INT;
  int num_inputs_chunk = num_inputs / 2;
  size_t size_inputs_total = sizeof(int) * num_inputs;
  size_t size_inputs_chunk = size_inputs_total / 2;

  // calculate workgroup offsets
  size_t goffset[3] = {0, 0, 0};
  size_t gdim[3], ldim[3]; // assuming three dimensions
  int ldim0 = 0;
  calculate_dimensions(&device, gdim, ldim, num_inputs_chunk, ldim0);

  // allocate the OpenCL input and result buffer memory objects on GPU device
  cl_mem buf_dev_inputs[2];
  buf_dev_inputs[0] = clCreateBuffer(ctx, CL_MEM_READ_ONLY, size_inputs_chunk, NULL, &err);
  if (err != CL_SUCCESS) printf("error: clCreateBuffer buf_dev_inputs 0: %d\n", err);
  buf_dev_inputs[1] = clCreateBuffer(ctx, CL_MEM_READ_ONLY, size_inputs_chunk, NULL, &err);
  if (err != CL_SUCCESS) printf("error: clCreateBuffer buf_dev_inputs 0: %d\n", err);

  cl_mem buf_dev_results[2];
  buf_dev_results[0] = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, size_inputs_total, NULL, &err);
  if (err != CL_SUCCESS) printf("error: clCreateBuffer buf_dev_results 0: %d\n", err);
  buf_dev_results[1] = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, size_inputs_total, NULL, &err);
  if (err != CL_SUCCESS) printf("error: clCreateBuffer buf_dev_results 0: %d\n", err);

  // allocate pinned input and result host buffers
  cl_mem pinned_host_inputs = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, size_inputs_total, NULL, &err);
  if (err != CL_SUCCESS) printf("error: clCreateBuffer pinned_host_inputs: %d\n", err);

  cl_mem pinned_host_results = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, size_inputs_total, NULL, &err);
  if (err != CL_SUCCESS) printf("error: clCreateBuffer pinned_host_results: %d\n", err);

  // get mapped pointers to pinned host buffers (to access them using standard pointers)
  int *inputs = (cl_int*)clEnqueueMapBuffer(queue[0], pinned_host_inputs, CL_TRUE, CL_MAP_WRITE, 0, size_inputs_total, 0, NULL, NULL, &err);
  if (err != CL_SUCCESS) printf("error: clEnqueueMapBuffer pinned_host_inputs: %d\n", err);

  int *results = (cl_int*)clEnqueueMapBuffer(queue[0], pinned_host_results, CL_TRUE, CL_MAP_READ, 0, size_inputs_total, 0, NULL, NULL, &err);
  if (err != CL_SUCCESS) printf("error: clEnqueueMapBuffer pinned_host_results: %d\n", err);

  // populate inputs
  for (int i = 0; i < num_inputs; i++) {
    inputs[i] = i;
  }

  // set kernel arguments for each kernel instance
  err = clSetKernelArg(knl[0], 0, sizeof(cl_mem), &buf_dev_inputs[0]);
  if (err != CL_SUCCESS) printf("error: clSetKernelArg 0: %d\n", err);
  err = clSetKernelArg(knl[0], 1, sizeof(cl_mem), &buf_dev_results[0]);
  if (err != CL_SUCCESS) printf("error: clSetKernelArg 1: %d\n", err);
  err = clSetKernelArg(knl[0], 2, sizeof(int), &num_inputs_chunk);
  if (err != CL_SUCCESS) printf("error: clSetKernelArg 2: %d\n", err);

  err = clSetKernelArg(knl[1], 0, sizeof(cl_mem), &buf_dev_inputs[1]);
  if (err != CL_SUCCESS) printf("error: clSetKernelArg 0: %d\n", err);
  err = clSetKernelArg(knl[1], 1, sizeof(cl_mem), &buf_dev_results[1]);
  if (err != CL_SUCCESS) printf("error: clSetKernelArg 1: %d\n", err);
  err = clSetKernelArg(knl[1], 2, sizeof(int), &num_inputs_chunk);
  if (err != CL_SUCCESS) printf("error: clSetKernelArg 2: %d\n", err);

  // flush queues before timing
  err = clFinish(queue[0]);
  if (err != CL_SUCCESS) printf("error: clFinish queue[0]: %d\n", err);
  err = clFinish(queue[1]);
  if (err != CL_SUCCESS) printf("error: clFinish queue[0]: %d\n", err);

  printf("trans-inputs\ttrans-results\texec-kernel\ttime-total\n");
  for (int j = 0; j < NUM_RUNS; j++) {
    // declare events
    cl_event event_inputs[NUM_CHUNKS];
    cl_event event_results[NUM_CHUNKS];
    cl_event event_kernel[NUM_CHUNKS];

    struct timespec ete_start, ete_end;
    get_timestamp(&ete_start);

    // Nonblocking write of 1st chunk of input data from host to device in queue 0
    err = clEnqueueWriteBuffer(queue[0], buf_dev_inputs[0], CL_FALSE, 0, size_inputs_chunk, (void*)&inputs[0], 0, NULL, &event_inputs[0]);
    if (err != CL_SUCCESS) printf("error: clEnqueueWriteBuffer buf_dev_inputs 0 queue 0: %d\n", err);

    // Launch kernel computation queue 0
    err = clEnqueueNDRangeKernel(queue[0], knl[0], 1, goffset, gdim, ldim, 1, &event_inputs[0], &event_kernel[0]);
    if (err != CL_SUCCESS) printf("error: clEnqueueNDRangeKernel queue 0: %d\n", err);

    // Nonblocking write of 2nd chunk of input data from host to device in queue 1
    err = clEnqueueWriteBuffer(queue[1], buf_dev_inputs[1], CL_FALSE, 0, size_inputs_chunk, (void*)&inputs[num_inputs_chunk], 0, NULL, &event_inputs[1]);
    if (err != CL_SUCCESS) printf("error: clEnqueueWriteBuffer buf_dev_inputs 1 queue 1: %d\n", err);

    // Launch kernel computation queue 1
    err = clEnqueueNDRangeKernel(queue[1], knl[1], 1, goffset, gdim, ldim, 1, &event_inputs[1], &event_kernel[1]);
    if (err != CL_SUCCESS) printf("error: clEnqueueNDRangeKernel queue 1: %d\n", err);

    // Nonblocking read of 1st chunk of results from device to host in queue 0
    err = clEnqueueReadBuffer(queue[0], buf_dev_results[0], CL_FALSE, 0, size_inputs_chunk, (void*)&results[0], 1, &event_kernel[0], &event_results[0]);
    if (err != CL_SUCCESS) printf("error: clEnqueueReadBuffer buf_dev_results 0 queue 0: %d\n", err);

    // Nonblocking read of 2nd chunk of results from device to host in queue 1
    err = clEnqueueReadBuffer(queue[1], buf_dev_results[1], CL_FALSE, 0, size_inputs_chunk, (void*)&results[num_inputs_chunk], 1, &event_kernel[1], &event_results[1]);
    if (err != CL_SUCCESS) printf("error: clEnqueueReadBuffer buf_dev_results 1 queue 0: %d\n", err);

    // Finish the queues
    err = clFinish(queue[0]);
    if (err != CL_SUCCESS) printf("error: clFinish queue[0]: %d\n", err);

    err = clFinish(queue[1]);
    if (err != CL_SUCCESS) printf("error: clFinish queue[1]: %d\n", err);

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
    for(int i = 0; i < 2; i++) {

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

      transfer_rate_inputs = (size_inputs_chunk * 0.001 * 0.001) / time_inputs;
      transfer_rate_results = (size_inputs_chunk * 0.001 * 0.001) / time_results;

      if(i == 1) {
        printf("%.6f\t%.6f\t%.6f\t%.6f\n", time_inputs, time_results, time_kernel, end_to_end);
      } else {
        printf("%.6f\t%.6f\t%.6f\n", time_inputs, time_results, time_kernel);
      }
    }
  }

  // Check results
  /*
  for(int i = 0; i < num_inputs; i++) {
    printf("%d: %d %d\n", i, inputs[i], results[i]);
  }
  */

  // Cleanup
    
  // Unmap pinned memory
  err = clEnqueueUnmapMemObject(queue[0], pinned_host_inputs, (void*)inputs, 0, NULL, NULL);
  if (err != CL_SUCCESS) printf("error: clEnqueueUnmapMemObject pinned_host_inputs: %d\n", err);
  err = clEnqueueUnmapMemObject(queue[0], pinned_host_results, (void*)results, 0, NULL, NULL);
  if (err != CL_SUCCESS) printf("error: clEnqueueUnmapMemObject pinned_host_results: %d\n", err);

  // Free memory buffers
  err = clReleaseMemObject(buf_dev_inputs[0]);
  if (err != CL_SUCCESS) printf("error: clReleaseMemObject buf_dev_inputs 0: %d\n", err);
  err = clReleaseMemObject(buf_dev_inputs[1]);
  if (err != CL_SUCCESS) printf("error: clReleaseMemObject buf_dev_inputs 1: %d\n", err);
  err = clReleaseMemObject(buf_dev_results[0]);
  if (err != CL_SUCCESS) printf("error: clReleaseMemObjec buf_dev_results 0: %d\n", err);
  err = clReleaseMemObject(buf_dev_results[1]);
  if (err != CL_SUCCESS) printf("error: clReleaseMemObjec buf_dev_results 1: %d\n", err);
  err = clReleaseMemObject(pinned_host_inputs);
  if (err != CL_SUCCESS) printf("error: clReleaseMemObject pinned_host_inputs: %d\n", err);
  err = clReleaseMemObject(pinned_host_results);
  if (err != CL_SUCCESS) printf("error: clReleaseMemObject pinned_host_results: %d\n", err);

  // Release command queues and context
  err = clReleaseKernel(knl[0]);
  err = clReleaseKernel(knl[1]);
  err = clReleaseCommandQueue(queue[0]);
  err = clReleaseCommandQueue(queue[1]);
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
