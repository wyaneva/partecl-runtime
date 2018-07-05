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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../kernel-gen/cpu-gen.h"
#include "../kernel-gen/structs.h"
#include "../kernel-gen/fsm.h"
#include "../kernel-gen/fsm.cl"
#include "../utils/options.h"
#include "../utils/read-test-cases.h"
#include "../utils/timing.h"
#include "../utils/utils.h"
#include "../utils/fsm-utils.h"
#include "../source/constants.h"

#if FSM_INPUTS_WITH_OFFSETS
int run_main(char *input, char *result, transition *transitions,
             short starting_state, int input_length, int output_length);

void run_on_cpu(char *input, char *result, transition *transitions,
                short starting_state, int input_length, int output_length) {

  run_main(input, result, transitions, starting_state, input_length,
           output_length);
}
#else
int run_main(struct partecl_input input, struct partecl_result *result,
             transition *transitions, short starting_state, int input_length,
             int output_length);

void run_on_cpu(struct partecl_input input, struct partecl_result *result,
                transition *transitions, short starting_state, int input_length,
                int output_length) {

  run_main(input, result, transitions, starting_state, input_length,
           output_length);
}
#endif

int main(int argc, char **argv) {

  print_sanity_checks();

  int do_print_results = HANDLE_RESULTS;
  int num_runs = NUM_RUNS;
  int do_time = DO_TIME;
  int num_test_cases = 1;
  char *filename = NULL;

  if (read_options(argc, argv, &num_test_cases, &do_print_results, &do_time,
                   &num_runs, NULL, NULL, NULL, NULL, &filename) == FAIL)
    return 0;
  printf("Device: CPU.\n");
  printf("Number of test cases %d.\n", num_test_cases);
#pragma omp parallel default(none)
  {
    int tid = omp_get_thread_num();
    if (tid == 0)
      printf("Number of threads = %d\n", omp_get_num_threads());
  }

  struct partecl_input *inputs;
  size_t inputs_size = sizeof(struct partecl_input) * num_test_cases;
  inputs = (struct partecl_input *)malloc(inputs_size);
  struct partecl_result *results;
  size_t results_size = sizeof(struct partecl_result) * num_test_cases;
  results = (struct partecl_result *)malloc(results_size);

  // read the test cases
  if (read_test_cases(inputs, num_test_cases) == FAIL)
    return 0;

  // execute the main code - TODO: plug it automatically
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

#if FSM_INPUTS_WITH_OFFSETS
  // calculate sizes
  int total_number_of_inputs;
  size_t size_inputs_offset;
  calculate_sizes_with_offset(&total_number_of_inputs, &size_inputs_offset, inputs, num_test_cases);

  // allocate memory
  char *inputs_offset = (char *)malloc(size_inputs_offset);
  char *results_offset = (char *)malloc(size_inputs_offset);
  int *offsets = (int *)malloc(sizeof(int) * num_test_cases);

  // copy data
  partecl_input_to_input_with_offsets(inputs, inputs_offset, offsets,
                                      num_test_cases);
#endif

  if (do_time)
    printf("Time in ms\n");

  for (int i = 0; i < num_runs; i++) {
    struct timespec time1, time2;
    get_timestamp(&time1);

#if FSM_INPUTS_WITH_OFFSETS
#pragma omp parallel for default(none)                                         \
    shared(num_test_cases, inputs_offset, results_offset, offsets, transitions, starting_state,       \
           input_length, output_length) schedule(static)
    for (int j = 0; j < num_test_cases; j++) {
      int offset = offsets[j];
      run_on_cpu(&inputs_offset[offset], &results_offset[offset], transitions, starting_state,
                 input_length, output_length);
    }
#else
#pragma omp parallel for default(none)                                         \
    shared(num_test_cases, inputs, results, transitions, starting_state,       \
           input_length, output_length) schedule(static)
    for (int j = 0; j < num_test_cases; j++) {
      run_on_cpu(inputs[j], &results[j], transitions, starting_state,
                 input_length, output_length);
    }
#endif

    get_timestamp(&time2);
    double time_in_secs = timestamp_diff_in_seconds(time1, time2);
    double time_cpu = time_in_secs * 1000;

    if (do_time)
      printf("%.6f \n", time_cpu);
  }

#if FSM_INPUTS_WITH_OFFSETS
  results_with_offsets_to_partecl_results(
      results_offset, results, total_number_of_inputs, offsets, num_test_cases);
#endif

  if (do_print_results)
    compare_results(results, NULL, num_test_cases);

  free(inputs);
  free(results);
}
