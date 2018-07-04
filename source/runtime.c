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

#include "../utils/options.h"
#include "../utils/read-test-cases.h"
#include "../utils/timing.h"
#include "../utils/utils.h"
#include "../utils/fsm-utils.h"
#include "cl-utils.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>
#include "../kernel-gen/cpu-gen.h"
#include "../kernel-gen/fsm.cl"
#include "../kernel-gen/fsm.h"
#include "constants.h"

#define GPU_SOURCE "../source/main-working.cl"
#define KERNEL_NAME "execute_fsm"

// IMPORTANT: kernel options should be defined in Makefile, based on the
// machine, on which we are compiling  otherwise, the kernel will not build
#ifndef KERNEL_OPTIONS
#define KERNEL_OPTIONS ""
#endif

void pad_test_case_number(cl_device_id *, int *);
void calculate_dimensions(cl_device_id *, size_t[3], size_t[3], int, int);
void calculate_global_offset(size_t[3], int, int);
void read_expected_results(struct partecl_result *, int);

int main(int argc, char **argv) {

  print_sanity_checks();

  // read command line options
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
                   &filename) == FAIL) {
    return 0;
  }

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

  // pad the test case number to nearest multiple of workgroup size
  //pad_test_case_number(&device, &num_test_cases);
  printf("Number of test cases: %d\n", num_test_cases);

  // check that the specified number of chunks divides the number of tests
  // equally
  if (num_test_cases % num_chunks != 0) {
    printf("Please provide a number of chunks which divides the padeed number "
           "of test cases (%d) equally.\n",
           num_test_cases);
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

  // read the test cases
  if (read_test_cases(inputs, num_test_cases) == FAIL)
    return 0;

  struct partecl_result *exp_results;
  exp_results = (struct partecl_result *)malloc(sizeof(struct partecl_result) *
                                                num_test_cases);
  if (do_compare_results)
    read_expected_results(exp_results, num_test_cases);

  // clalculate dimensions
  size_t gdim[3], ldim[3]; // assuming three dimensions
  calculate_dimensions(&device, gdim, ldim, chunksize, ldim0);
  printf("LDIM = %zd, chunks = %d\n", ldim[0], num_chunks);

  // execute main code from FSM (TODO: plug main code from source file)
  int num_transitions;
  int starting_state;
  int input_length;
  int output_length;
  if (filename == NULL) {
    printf("Please provide an FSM filename.\n");
    return 0;
  }
  printf("Reading fsm: %s\n", filename);
  transition *transitions =
      read_fsm(filename, &num_transitions, &starting_state, &input_length, &output_length);

  if (transitions == NULL) {
    printf("Reading the FSM failed.");
    return -1;
  }

  size_t size_transitions = sizeof(transition) * NUM_STATES * MAX_NUM_TRANSITIONS_PER_STATE;
  printf("Size of FSM with %d transitions is %ld bytes.\n", num_transitions,
         size_transitions);

#if FSM_INPUTS_WITH_OFFSETS
  // calculate sizes
  int total_number_of_inputs;
  size_t size_inputs_offset;
  calculate_sizes_with_offset(&total_number_of_inputs, &size_inputs_offset,
                              inputs, num_test_cases);

  // allocate memory
  char *inputs_offset = (char *)malloc(size_inputs_offset);
  char *results_offset = (char *)malloc(size_inputs_offset);
  int *offsets = (int *)malloc(sizeof(int) * num_test_cases);

  // copy data
  partecl_input_to_input_with_offsets(inputs, inputs_offset, offsets,
                                      num_test_cases);

  printf("Size of %d test inputs is %ld bytes.\n", num_test_cases,
         size_inputs_offset);
  printf("Size of %d test results is %ld bytes.\n", num_test_cases,
         size_inputs_offset);
#else
#if FSM_INPUTS_COAL_CHAR
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
  char *results_coal = (char *)malloc(size_inputs_coal);
  printf("Size of %d test inputs is %ld bytes.\n", num_test_cases,
         size_inputs_coal);
  printf("Size of %d test results is %ld bytes.\n", num_test_cases,
         size_inputs_coal);
#else
#if FSM_INPUTS_COAL_CHAR4
  // TODO: NOTE WE ARE NOT TAKING INPUT LENGTH INTO ACCOUNT HERE
  int padded_size =
      PADDED_INPUT_ARRAY_SIZE + CHAR_N - PADDED_INPUT_ARRAY_SIZE % CHAR_N;
  size_t size_inputs_coal_char4 =
      sizeof(cl_char4) * padded_size * num_test_cases / CHAR_N;
  cl_char4 *inputs_coal_char4 = (cl_char4 *)malloc(size_inputs_coal_char4);
  int max_num_inputs =
      PADDED_INPUT_ARRAY_SIZE /
      input_length; // this is the maximum number of inputs per test case
  for (int i = 0; i < max_num_inputs;
       i += CHAR_N) { // which input inside the test case
    for (int j = 0; j < num_test_cases; j++) { // which test case
      struct partecl_input current_input = inputs[j];

      size_t idx = (i / CHAR_N) * num_test_cases + j;
      for (int k = 0; k < CHAR_N; k++) {
        char current_symbol = current_input.input_ptr[i + k];
        inputs_coal_char4[idx].s[k] = current_symbol;
      }
    }
  }
#else
  printf("Size of %d test inputs is %ld bytes.\n", num_test_cases, size_inputs);
  printf("Size of %d test results is %ld bytes.\n", num_test_cases,
         size_results);
#endif
#endif
#endif

  // create kernel
  char *knl_text = read_file(GPU_SOURCE);
  if (!knl_text) {
    printf("Couldn't read file %s. Exiting!\n", GPU_SOURCE);
    return -1;
  }

  // build the kernel options
  char kernel_options[1000];
  char *kernel_options_ptr = &kernel_options[0];
  kernel_options_ptr = concatenate_strings(kernel_options_ptr, KERNEL_OPTIONS);
  char ko_num_transitions[50];
  sprintf(ko_num_transitions, " -DNUM_TRANSITIONS_KERNEL=%d", num_transitions);
  kernel_options_ptr = concatenate_strings(kernel_options_ptr, ko_num_transitions);

  // will fsm fit in constant or local memory?
#if FSM_OPTIMISE_CONST_MEM
  int enough_constant_memory =
      size_transitions > get_constant_mem_size(&device) ? 0 : 1;
#else
  int enough_constant_memory = 0;
#endif

  cl_kernel knl;
  if (enough_constant_memory) { // first try to fit in constant memory
    kernel_options_ptr = concatenate_strings(kernel_options_ptr, " -DFSM_CONSTANT_MEMORY=1");
    knl = kernel_from_string(ctx, knl_text, KERNEL_NAME, kernel_options);

  } else { // try to fit into local memory
    int enough_local_memory =
        size_transitions > get_local_mem_size(&device) ? 0 : 1;

    if (enough_local_memory) {
      kernel_options_ptr = concatenate_strings(kernel_options_ptr, " -DFSM_LOCAL_MEMORY=1");
      knl = kernel_from_string(ctx, knl_text, KERNEL_NAME, kernel_options);

    } else {
      knl = kernel_from_string(ctx, knl_text, KERNEL_NAME, kernel_options);
    }
  }
  free(knl_text);

  if (do_time) {
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
#if FSM_INPUTS_WITH_OFFSETS
    cl_mem buf_inputs =
        clCreateBuffer(ctx, CL_MEM_READ_WRITE, size_inputs_offset, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer buf_inputs: %d\n", err);

    cl_mem buf_results =
        clCreateBuffer(ctx, CL_MEM_READ_WRITE, size_inputs_offset, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer buf_results: %d\n", err);
#else
#if FSM_INPUTS_COAL_CHAR
    cl_mem buf_inputs =
        clCreateBuffer(ctx, CL_MEM_READ_WRITE, size_inputs_coal, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer buf_inputs: %d\n", err);

    cl_mem buf_results =
        clCreateBuffer(ctx, CL_MEM_READ_WRITE, size_inputs_coal, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer buf_results: %d\n", err);
#else
#if FSM_INPUTS_COAL_CHAR4
    cl_mem buf_inputs = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
                                       size_inputs_coal_char4, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer buf_inputs: %d\n", err);
#else
    cl_mem buf_inputs =
        clCreateBuffer(ctx, CL_MEM_READ_WRITE, size_inputs, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer buf_inputs: %d\n", err);

    cl_mem buf_results =
        clCreateBuffer(ctx, CL_MEM_READ_WRITE, size_results, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer buf_results: %d\n", err);
#endif
#endif
#endif

#if FSM_INPUTS_WITH_OFFSETS
    cl_mem buf_offsets =
        clCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(int) * num_test_cases, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer buf_offsets: %d\n", err);
#endif

    cl_mem buf_transitions =
        clCreateBuffer(ctx, CL_MEM_READ_ONLY, size_transitions, NULL, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateBuffer buf_transitions: %d\n", err);

#if FSM_INPUTS_WITH_OFFSETS
#define KNL_ARG_TRANSITIONS 3
#define KNL_ARG_STARTING_STATE 4
#define KNL_ARG_INPUT_LENGTH 5
#define KNL_ARG_OUTPUT_LENGTH 6
#define KNL_ARG_NUM_TEST_CASES 7
#else
#define KNL_ARG_TRANSITIONS 2
#define KNL_ARG_STARTING_STATE 3
#define KNL_ARG_INPUT_LENGTH 4
#define KNL_ARG_OUTPUT_LENGTH 5
#define KNL_ARG_NUM_TEST_CASES 6
#endif

    // add kernel arguments
    err = clSetKernelArg(knl, 0, sizeof(cl_mem), &buf_inputs);
    if (err != CL_SUCCESS)
      printf("error: clSetKernelArg 0: %d\n", err);

    err = clSetKernelArg(knl, 1, sizeof(cl_mem), &buf_results);
    if (err != CL_SUCCESS)
      printf("error: clSetKernelArg 1: %d\n", err);

#if FSM_INPUTS_WITH_OFFSETS
    err = clSetKernelArg(knl, 2, sizeof(cl_mem), &buf_offsets);
    if (err != CL_SUCCESS)
      printf("error: clSetKernelArg 2: %d\n", err);
#endif

    err = clSetKernelArg(knl, KNL_ARG_TRANSITIONS, sizeof(cl_mem), &buf_transitions);
    if (err != CL_SUCCESS)
      printf("error: clSetKernelArg %d: %d\n", KNL_ARG_TRANSITIONS, err);

    err = clSetKernelArg(knl, KNL_ARG_STARTING_STATE, sizeof(int), &starting_state);
    if (err != CL_SUCCESS)
      printf("error: clSetKernelArg %d: %d\n", KNL_ARG_STARTING_STATE, err);

    err = clSetKernelArg(knl, KNL_ARG_INPUT_LENGTH, sizeof(int), &input_length);
    if (err != CL_SUCCESS)
      printf("error: clSetKernelArg %d: %d\n", KNL_ARG_INPUT_LENGTH, err);

    err = clSetKernelArg(knl, KNL_ARG_OUTPUT_LENGTH, sizeof(int), &output_length);
    if (err != CL_SUCCESS)
      printf("error: clSetKernelArg %d: %d\n", KNL_ARG_OUTPUT_LENGTH, err);

    err = clSetKernelArg(knl, KNL_ARG_NUM_TEST_CASES, sizeof(int), &num_test_cases);
    if (err != CL_SUCCESS)
      printf("error: clSetKernelArg %d: %d\n", KNL_ARG_NUM_TEST_CASES, err);

    // declare events
    cl_event event_inputs[num_chunks];
    cl_event event_offsets[num_chunks];
    cl_event event_kernel[num_chunks];
    cl_event event_results[num_chunks];

    // flush the queues before timing
    err = clFinish(queue_inputs);
    if (err != CL_SUCCESS)
      printf("error: clFinish queue_inputs: %d\n", err);

    err = clFinish(queue_kernel);
    if (err != CL_SUCCESS)
      printf("error: clFinish queue_kernel: %d\n", err);

    err = clFinish(queue_results);
    if (err != CL_SUCCESS)
      printf("error: clFinish queue_results: %d\n", err);

    get_timestamp(&ete_start);

    // transfer FSM to GPU only once
    cl_event event_fsm;
    err = clEnqueueWriteBuffer(queue_inputs, buf_transitions, CL_TRUE, 0,
                               size_transitions, transitions, 0, NULL,
                               &event_fsm);
    if (err != CL_SUCCESS)
      printf("error: clEnqueueWriteBuffer buf_transitions: %d\n", err);

    for (int j = 0; j < num_chunks; j++) {
      // transfer input to device
#if FSM_INPUTS_WITH_OFFSETS
      err = clEnqueueWriteBuffer(
          queue_inputs, buf_offsets, CL_FALSE, sizeof(int) * chunksize * j,
          sizeof(int) * num_test_cases / num_chunks, offsets + chunksize * j, 0,
          NULL, &event_offsets[j]);
      if (err != CL_SUCCESS)
        printf("error: clEnqueueWriteBuffer %d: %d\n", j, err);

      err = clEnqueueWriteBuffer(
          queue_inputs, buf_inputs, CL_FALSE, sizeof(char) * chunksize * j,
          size_inputs_offset / num_chunks, inputs_offset + chunksize * j, 1,
          &event_offsets[j], &event_inputs[j]);
      if (err != CL_SUCCESS)
        printf("error: clEnqueueWriteBuffer %d: %d\n", j, err);

#else
#if FSM_INPUTS_COAL_CHAR
      err = clEnqueueWriteBuffer(
          queue_inputs, buf_inputs, CL_FALSE, sizeof(char) * chunksize * j,
          size_inputs_coal / num_chunks, inputs_coal + chunksize * j, 0, NULL,
          &event_inputs[j]);
      if (err != CL_SUCCESS)
        printf("error: clEnqueueWriteBuffer %d: %d\n", j, err);
#else
#if FSM_INPUTS_COAL_CHAR4
      err = clEnqueueWriteBuffer(
          queue_inputs, buf_inputs, CL_FALSE, sizeof(cl_char4) * chunksize * j,
          size_inputs_coal_char4 / num_chunks,
          inputs_coal_char4 + chunksize * j, 0, NULL, &event_inputs[j]);
      if (err != CL_SUCCESS)
        printf("error: clEnqueueWriteBuffer %d: %d\n", j, err);
#else
      err = clEnqueueWriteBuffer(
          queue_inputs, buf_inputs, CL_FALSE,
          sizeof(partecl_input) * chunksize * j, size_inputs / num_chunks,
          inputs + chunksize * j, 0, NULL, &event_inputs[j]);
      if (err != CL_SUCCESS)
        printf("error: clEnqueueWriteBuffer %d: %d\n", j, err);
#endif
#endif
#endif

      // launch kernel
      size_t goffset[3];
      calculate_global_offset(goffset, chunksize, j);

      err = clEnqueueNDRangeKernel(queue_kernel, knl, 1, goffset, gdim, ldim, 1,
                                   &event_inputs[j], &event_kernel[j]);
      if (err != CL_SUCCESS)
        printf("error: clEnqueueNDRangeKernel %d: %d\n", j, err);

      // transfer results back
#if FSM_INPUTS_WITH_OFFSETS
      err = clEnqueueReadBuffer(
          queue_results, buf_results, CL_FALSE, sizeof(char) * chunksize * j,
          size_inputs_offset / num_chunks, results_offset + chunksize * j, 1,
          &event_kernel[j], &event_results[j]);
      if (err != CL_SUCCESS)
        printf("error: clEnqueueReadBuffer %d: %d\n", j, err);
#else
#if FSM_INPUTS_COAL_CHAR
      err = clEnqueueReadBuffer(
          queue_results, buf_results, CL_FALSE, sizeof(char) * chunksize * j,
          size_inputs_coal / num_chunks, results_coal + chunksize * j, 1,
          &event_kernel[j], &event_results[j]);
      if (err != CL_SUCCESS)
        printf("error: clEnqueueReadBuffer %d: %d\n", j, err);
#else
#if FSM_INPUTS_COAL_CHAR4
      // TODO
#else
      err = clEnqueueReadBuffer(queue_results, buf_results, CL_FALSE,
                                sizeof(struct partecl_result) * chunksize * j,
                                size_results / num_chunks,
                                results + chunksize * j, 1, &event_kernel[j],
                                &event_results[j]);
      if (err != CL_SUCCESS)
        printf("error: clEnqueueReadBuffer %d: %d\n", j, err);
#endif
#endif
#endif
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

    err = clReleaseMemObject(buf_transitions);
    if (err != CL_SUCCESS)
      printf("error: clReleaseMemObjec: %d\n", err);

#if FSM_INPUTS_WITH_OFFSETS
    err = clReleaseMemObject(buf_offsets);
    if (err != CL_SUCCESS)
      printf("error: clReleaseMemObjec: %d\n", err);
#endif

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

      if (do_time) {
        if(j == num_chunks - 1) {
          end_to_end = timestamp_diff_in_seconds(ete_start, ete_end) * 1000; // in ms
          printf("%.6f\t%.6f\t%.6f\t%.6f\n", trans_inputs, trans_results, time_gpu,
               end_to_end);
        }
        /*
        else {
          printf("%.6f\t%.6f\t%.6f\n", trans_inputs, trans_results, time_gpu);
        }
        */
      }
    }

#if FSM_INPUTS_WITH_OFFSETS
    results_with_offsets_to_partecl_results(results_offset, results, total_number_of_inputs, offsets, num_test_cases);
#endif

#if FSM_INPUTS_COAL_CHAR
    for (int i = 0; i < num_test_cases; i++) {
      char* outputptr = results[i].output;
      for (int j = i; j < PADDED_INPUT_ARRAY_SIZE*num_test_cases; j += num_test_cases) {
        *outputptr = results_coal[j];
        outputptr++;
      }
      results[i].length = strlen(results[i].output);
    }
#endif

#if FSM_INPUTS_COAL_CHAR4
#endif

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
#if FSM_INPUTS_COAL_CHAR
  free(inputs_coal);
#endif
#if FSM_INPUTS_COAL_CHAR4
  free(inputs_coal_char4);
#endif
#if FSM_INPUTS_WITH_OFFSETS
  free(inputs_offset);
  free(results_offset);
  free(offsets);
#endif
}

void pad_test_case_number(cl_device_id *device, int *num_test_cases) {
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

  if (*num_test_cases % dims[0] != 0) {
    int coef = *num_test_cases / dims[0];
    *num_test_cases = (coef + 1) * dims[0];
  }
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

