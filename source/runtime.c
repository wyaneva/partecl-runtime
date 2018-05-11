/*
 * Copyright 2016-2018 Vanya Yaneva, The University of Edinburgh
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../kernel-gen/cpu-gen.h"
#include "../kernel-gen/fsm.cl"
#include "../kernel-gen/fsm.h"
#include "../utils/options.h"
#include "../utils/read-test-cases.h"
#include "../utils/timing.h"
#include "../utils/utils.h"
#include "cl-utils.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GPU_SOURCE "../source/main-working.cl"
#define KERNEL_NAME "execute_fsm"

// IMPORTANT: kernel options should be defined in Makefile, based on the
// machine, on which we are compiling  otherwise, the kernel will not build
#ifndef KERNEL_OPTIONS
#define KERNEL_OPTIONS "NONE"
#endif

void calculate_dimensions(cl_device_id *, size_t[3], size_t[3], int, int);
void calculate_global_offset(size_t[3], int, int);
void read_expected_results(struct partecl_result *, int);

void setup_common_buffers(cl_context ctx, cl_kernel knl,
                          cl_command_queue queue_inputs,
                          struct transition *transitions, int num_transitions,
                          int input_length, int output_length,
                          int num_test_cases) {
  // setup buffers
  size_t size_transitions = sizeof(struct transition) * num_transitions;
  printf("Size of FSM with %d transitions is %ld bytes.\n", num_transitions,
         size_transitions);
  cl_int err;
  cl_mem buf_transitions =
      clCreateBuffer(ctx, CL_MEM_READ_WRITE, size_transitions, NULL, &err);
  if (err != CL_SUCCESS)
    printf("error: clCreateBuffer buf_transitions: %d\n", err);

  // transfer data to GPU
  cl_event event_common_buffers;
  err = clEnqueueWriteBuffer(queue_inputs, buf_transitions, CL_FALSE, 0,
                             size_transitions, transitions, 0, NULL,
                             &event_common_buffers);
  if (err != CL_SUCCESS)
    printf("error: clEnqueueWriteBuffer buf_transitions: %d\n", err);

  // set kernels
  err = clSetKernelArg(knl, 2, sizeof(cl_mem), &buf_transitions);
  if (err != CL_SUCCESS)
    printf("error: clSetKernelArg 2: %d\n", err);

  err = clSetKernelArg(knl, 3, sizeof(int), &num_transitions);
  if (err != CL_SUCCESS)
    printf("error: clSetKernelArg 3: %d\n", err);

  err = clSetKernelArg(knl, 4, sizeof(int), &input_length);
  if (err != CL_SUCCESS)
    printf("error: clSetKernelArg 4: %d\n", err);

  err = clSetKernelArg(knl, 5, sizeof(int), &output_length);
  if (err != CL_SUCCESS)
    printf("error: clSetKernelArg 5: %d\n", err);

  err = clSetKernelArg(knl, 6, sizeof(int), &num_test_cases);
  if (err != CL_SUCCESS)
    printf("error: clSetKernelArg 6: %d\n", err);
}

int main(int argc, char **argv) {
  int do_compare_results = HANDLE_RESULTS;
  int num_runs = NUM_RUNS;
  int do_time = DO_TIME;
  int ldim0 = LDIM;
  int do_choose_device = DO_CHOOSE_DEVICE;
  int num_chunks = NUM_CHUNKS;
  int num_test_cases = 1;
  char *filename = NULL;

  if (read_options(argc, argv, &num_test_cases, &do_compare_results, &do_time,
                   &num_runs, &ldim0, &do_choose_device, &num_chunks,
                   &filename) == FAIL)
    return 0;

  // check that the specified number of chunks divides the number of tests
  // equally
  if (num_test_cases % num_chunks != 0) {
    printf("Please provide a number of chunks which divides the number of test "
           "cases equally.\n");
    return 0;
  }
  int chunksize = num_test_cases / num_chunks;

  // allocate CPU memory and generate test cases
  struct partecl_input *inputs;
  size_t size_inputs = sizeof(struct partecl_input) * num_test_cases;
  inputs = (struct partecl_input *)malloc(size_inputs);
  struct partecl_result *results;
  size_t size_results = sizeof(struct partecl_result) * num_test_cases;
  results = (struct partecl_result *)malloc(size_results);

  // create queue and context
  cl_context ctx;
  cl_command_queue queue_inputs;
  cl_command_queue queue_kernel;
  cl_command_queue queue_results;
  cl_int err;
  cl_device_id device;
  create_context_on_gpu(&ctx, &device, do_choose_device);
  create_command_queue(&queue_inputs, &ctx, &device);
  create_command_queue(&queue_kernel, &ctx, &device);
  create_command_queue(&queue_results, &ctx, &device);

  // read the test cases
  if (read_test_cases(inputs, num_test_cases) == FAIL)
    return 0;

  // create kernel
  char *knl_text = read_file(GPU_SOURCE);
  if (!knl_text) {
    printf("Couldn't read file %s. Exiting!\n", GPU_SOURCE);
    return -1;
  }

  cl_kernel knl =
      kernel_from_string(ctx, knl_text, KERNEL_NAME, KERNEL_OPTIONS);
  free(knl_text);

  struct partecl_result *exp_results;
  exp_results = (struct partecl_result *)malloc(sizeof(struct partecl_result) *
                                                num_test_cases);
  if (do_compare_results)
    read_expected_results(exp_results, num_test_cases);

  // clalculate dimensions
  size_t gdim[3], ldim[3]; // assuming three dimensions
  calculate_dimensions(&device, gdim, ldim, chunksize, ldim0);
  printf("LDIM = %zd\n", ldim[0]);

  // execute main code from FSM (TODO: plug main code from source file)
  int num_transitions;
  int input_length;
  int output_length;
  if (filename == NULL) {
    printf("Please provide an FSM filename.\n");
    return 0;
  }
  printf("Reading fsm: %s\n", filename);
  struct transition *transitions =
      read_fsm(filename, &num_transitions, &input_length, &output_length);
  setup_common_buffers(ctx, knl, queue_inputs, transitions, num_transitions,
                       input_length, output_length, num_test_cases);

  // transpose inputs for coalesced reading on gpu
  // TODO: This might be a potentially automatically generated code, as it
  // depends on the name of the variable in side the input structure
  // i = which input inside the test case
  // j = which test case
  // k = which symbol inside the input
  size_t size_inputs_coal =
      sizeof(char) * PADDED_INPUT_ARRAY_SIZE * num_test_cases;
  char *inputs_coal = (char *)malloc(size_inputs_coal);
  int max_num_inputs =
      PADDED_INPUT_ARRAY_SIZE /
      input_length; // this is the maximum number of inputs per test case
  for (int i = 0; i < max_num_inputs; i++) { // which input inside the test case
    for (int j = 0; j < num_test_cases; j++) { // which test case
      struct partecl_input current_input = inputs[j];
      for (int k = 0; k < input_length; k++) {
        size_t idx = (i * num_test_cases + j) * input_length + k;
        char current_symbol = current_input.input_ptr[i * input_length + k];
        inputs_coal[idx] = current_symbol;
      }
    }
  }

  if (do_time) {
    printf("Number of test cases: %d\n", num_test_cases);
    printf("Time in ms\n");
    printf("trans-inputs\ttrans-results\texec-kernel\ttime-total\n");
  }

  for (int i = 0; i < num_runs; i++) {
    // timing variables
    double trans_inputs = 0.0;
    double trans_results = 0.0;
    double time_gpu = 0.0;
    double end_to_end = 0.0;
    struct timespec ete_start, ete_end;
    cl_ulong ev_start_time, ev_end_time;

    // allocate device memory
    cl_mem buf_inputs =
        clCreateBuffer(ctx, CL_MEM_READ_WRITE, size_inputs_coal, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer: %d\n", err);

    cl_mem buf_results =
        clCreateBuffer(ctx, CL_MEM_READ_WRITE, size_results, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer: %d\n", err);

    // declare events
    cl_event event_inputs[num_chunks];
    cl_event event_kernel[num_chunks];
    cl_event event_results[num_chunks];

    get_timestamp(&ete_start);

    for (int j = 0; j < num_chunks; j++) {
      // transfer input to device
      err = clEnqueueWriteBuffer(
          queue_inputs, buf_inputs, CL_FALSE, sizeof(char) * chunksize * j,
          size_inputs_coal / num_chunks, inputs_coal + chunksize * j, 0, NULL,
          &event_inputs[j]);
      if (err != CL_SUCCESS)
        printf("error: clEnqueueWriteBuffer %d: %d\n", j, err);

      // add kernel arguments
      err = clSetKernelArg(knl, 0, sizeof(cl_mem), &buf_inputs);
      if (err != CL_SUCCESS)
        printf("error: clSetKernelArg 0: %d\n", err);

      err = clSetKernelArg(knl, 1, sizeof(cl_mem), &buf_results);
      if (err != CL_SUCCESS)
        printf("error: clSetKernelArg 1: %d\n", err);

      // launch kernel
      size_t goffset[3];
      calculate_global_offset(goffset, chunksize, j);

      err = clEnqueueNDRangeKernel(queue_kernel, knl, 1, goffset, gdim, ldim, 1,
                                   &event_inputs[j], &event_kernel[j]);
      // err = clEnqueueNDRangeKernel(queue, knl, 1, NULL, gdim, ldim, 0, NULL,
      // NULL);
      if (err != CL_SUCCESS)
        printf("error: clEnqueueNDRangeKernel %d: %d\n", j, err);

      // transfer results back
      err = clEnqueueReadBuffer(
          queue_results, buf_results, CL_FALSE,
          sizeof(partecl_result) * chunksize * j, size_results / num_chunks,
          results + chunksize * j, 1, &event_kernel[j], &event_results[j]);
      if (err != CL_SUCCESS)
        printf("error: clEnqueueReadBuffer %d: %d\n", j, err);
    }

    // finish the kernels
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

    // gather performance data
    for (int j = 0; j < num_chunks; j++) {
      clGetEventProfilingInfo(event_inputs[j], CL_PROFILING_COMMAND_START,
                              sizeof(cl_ulong), &ev_start_time, NULL);
      clGetEventProfilingInfo(event_inputs[j], CL_PROFILING_COMMAND_END,
                              sizeof(cl_ulong), &ev_end_time, NULL);
      trans_inputs = (double)(ev_end_time - ev_start_time) / 1000000;

      clGetEventProfilingInfo(event_results[j], CL_PROFILING_COMMAND_START,
                              sizeof(cl_ulong), &ev_start_time, NULL);
      clGetEventProfilingInfo(event_results[j], CL_PROFILING_COMMAND_END,
                              sizeof(cl_ulong), &ev_end_time, NULL);
      trans_results = (double)(ev_end_time - ev_start_time) / 1000000;

      clGetEventProfilingInfo(event_kernel[j], CL_PROFILING_COMMAND_START,
                              sizeof(cl_ulong), &ev_start_time, NULL);
      clGetEventProfilingInfo(event_kernel[j], CL_PROFILING_COMMAND_END,
                              sizeof(cl_ulong), &ev_end_time, NULL);
      time_gpu = (double)(ev_end_time - ev_start_time) / 1000000;
    }

    end_to_end = timestamp_diff_in_seconds(ete_start, ete_end) * 1000; // in ms
    if (do_time)
      printf("%.6f\t%.6f\t%.6f\t%.6f\n", trans_inputs, trans_results, time_gpu,
             end_to_end);

    // check results
    if (do_compare_results)
      compare_results(results, exp_results, num_test_cases);

    for (int j = 0; j < num_chunks; j++) {
      err = clReleaseEvent(event_inputs[j]);
      if (err != CL_SUCCESS)
        printf("error: clReleaseEvent (event_inputs) %d: %d\n", j, err);
      err = clReleaseEvent(event_results[j]);
      if (err != CL_SUCCESS)
        printf("error: clReleaseEvent (event_results) %d: %d\n", j, err);
      err = clReleaseEvent(event_kernel[j]);
      if (err != CL_SUCCESS)
        printf("error: clReleaseEvent (event_kernel) %d: %d\n", j, err);
    }
  }

  free(inputs);
  free(results);
  free(exp_results);
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

  if (ldimsupplied != LDIM) {
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

void calculate_global_offset(size_t goffset[3], int chunksize, int j) {
  goffset[0] = chunksize * j;
  goffset[1] = 0;
  goffset[2] = 0;
}

void read_expected_results(struct partecl_result *results, int num_test_cases) {
  // TODO:
}

