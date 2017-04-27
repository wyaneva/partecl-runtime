/*
 * Copyright 2016 Vanya Yaneva, The University of Edinburgh
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "../utils/options.h"
#include "../utils/read-test-cases.h"
#include "../utils/timing.h"
#include "../utils/utils.h"
#include "../kernel-gen/cpu-gen.h"
#include "kernel.h"

#define GPU_SOURCE "../kernel-gen/test.cl"
#define KERNEL_NAME "main_kernel"

//IMPORTANT: kernel options should be defined in Makefile, based on the machine, on which we are compiling
//otherwise, the kernel will not build
#ifndef KERNEL_OPTIONS
#define KERNEL_OPTIONS "NONE"
#endif

void calculate_global_offset(size_t[3], int, int);
void read_expected_results(struct partecl_result *, int);

int main(int argc, char **argv)
{
  int do_compare_results = HANDLE_RESULTS;
  int num_runs = NUM_RUNS;
  int do_time = DO_TIME;
  int ldim0 = LDIM;
  int do_choose_device = DO_CHOOSE_DEVICE;
  int num_chunks = NUM_CHUNKS;
  int num_test_cases = 1;

  if(read_options(argc, argv, &num_test_cases, &do_compare_results, &do_time, &num_runs, &ldim0, &do_choose_device, &num_chunks) == FAIL)
    return 0;

  //check that the specified number of chunks divides the number of tests equally
  if(num_test_cases % num_chunks != 0)
  {
    printf("Please provide a number of chunks which divides the number of test cases equally.\n");
    return 0;
  }

  //allocate CPU memory and generate test cases
  struct partecl_input * inputs;
  size_t size_inputs = sizeof(struct partecl_input) * num_test_cases;
  inputs = (struct partecl_input*)malloc(size_inputs);
  struct partecl_result * results;
  size_t size_results = sizeof(struct partecl_result) * num_test_cases;
  results = (struct partecl_result *)malloc(size_results);

  //read the test cases
  if(read_test_cases(inputs, num_test_cases) == FAIL)
    return 0;

  //read the expected results
  struct partecl_result * exp_results;
  exp_results = (struct partecl_result *)malloc(sizeof(struct partecl_result)*num_test_cases);
  if(do_compare_results)
    read_expected_results(exp_results, num_test_cases);

  //run the kernel code
  exec_kernel(
      do_compare_results,
      num_runs,
      do_time,
      ldim0,
      num_chunks,
      num_test_cases,
      inputs,
      results,
      size_inputs,
      size_results);

  //check results
  if(do_compare_results)
    compare_results(results, exp_results, num_test_cases);

  free(inputs);
  free(results);
  free(exp_results);
}

void calculate_global_offset(size_t goffset[3], int chunksize, int j)
{
  goffset[0] = chunksize*j;
  goffset[1] = 0;
  goffset[2] = 0;
}

void read_expected_results(struct partecl_result *results, int num_test_cases)
{
  //TODO:
}

