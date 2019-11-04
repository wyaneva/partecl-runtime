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

#if BMRK_C
int run_main(struct partecl_input input, struct partecl_result *result,
             transition *transitions, short starting_state, int input_length,
             int output_length);

void run_on_cpu(struct partecl_input input, struct partecl_result *result,
                transition *transitions, short starting_state, int input_length,
                int output_length) {

  run_main(input, result, transitions, starting_state, input_length,
           output_length);
}
#else 
int run_main(char *input, char *result, transition *transitions,
             short starting_state, int input_length, int output_length);

void run_on_cpu(char *input, char *result, transition *transitions,
                short starting_state, int input_length, int output_length) {

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
  int do_sort_test_cases = DO_SORT_TEST_CASES;
  int size_chunks = SIZE_CHUNKS;
  char *filename = NULL;

  if (read_options(argc, argv, &num_test_cases, &do_print_results, &do_time,
                   &num_runs, NULL, NULL, &size_chunks, NULL, &do_sort_test_cases,
                   &filename) == FAIL)
    return 0;
  printf("Device: CPU.\n");
  printf("Number of test cases %d.\n", num_test_cases);
#pragma omp parallel default(none)
  {
    int tid = omp_get_thread_num();
    if (tid == 0)
      printf("Number of threads = %d\n", omp_get_num_threads());
  }

  // chunk parameters
  size_chunks *= KB_TO_B; // turn into bytes
  int num_tests_chunk = 0;
  size_t size_inputs_chunk = 0;
  int padded_input_size_chunk = 0;

  struct aggr aggregate = {0, 0.0, 0.0};
#if BMRK_C || FSM_INPUTS_WITH_OFFSETS //testing C programs or fsm with offsets
  struct partecl_input *inputs;
  size_t inputs_size = sizeof(struct partecl_input) * num_test_cases;
  inputs = (struct partecl_input *)malloc(inputs_size);
  struct partecl_result *results;
  size_t results_size = sizeof(struct partecl_result) * num_test_cases;
  results = (struct partecl_result *)malloc(results_size);

  // read the test cases
  if (read_test_cases(inputs, num_test_cases, &aggregate) == FAIL) {
    printf("Failed reading the test cases.\n");
    return -1;
  }
#else //testing FSMs

  char *inputs[num_test_cases];
  char *results[num_test_cases];

  // read test cases
  int test_index_total = 0;
  int test_id_start = 0;
  int test_id_end;
  read_test_cases_chunk(inputs, num_test_cases, &test_index_total, size_chunks, test_id_start, &aggregate, &test_id_end);

  for(int i=0; i<num_test_cases; i++) {
    results[i] = (char *)malloc(strlen(inputs[i])*sizeof(char*));
  }
#endif

#if DO_TEST_DIST
  // write the aggregate in the test analysis file
  double mean = 0.0;
  double sd = 0.0;
  finalise_aggr(aggregate, &mean, &sd);

  // find out the fsm name from the filename
  char *fsmfilename = (char*)malloc(sizeof(char)*500);
  char *fsmname = (char*)malloc(sizeof(char)*15);
  char *token;

  strcpy(fsmfilename, filename);
  while((token = strsep(&fsmfilename, "//")) != NULL) {
    strcpy(fsmname, token);
  }

  // open file to put averages in
  FILE *analysis_f = fopen(TEST_DIST_FILE, "a");
  if (analysis_f == NULL) {
    printf("Error opening file %s!\n", TEST_DIST_FILE);
    return 0;
  }

  fprintf(analysis_f, "%s,%d,%f,%f\n", fsmname, num_test_cases, mean, sd);
  fclose(analysis_f);

  free(fsmfilename);
  free(fsmname);
#endif

  /*
  if (do_sort_test_cases) {
    // sort the test cases
    if (sort_test_cases_by_length(inputs, num_test_cases, SORT_ASCENDING) == FAIL) {
      printf("Failed sorting the test cases by length.\n");
      return -1;
    }
  }
  */

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

#if BMRK_C //testing C programs

#pragma omp parallel for default(none)                                         \
    shared(num_test_cases, inputs, results, transitions, starting_state,       \
           input_length, output_length) schedule(guided)
    for (int j = 0; j < num_test_cases; j++) {
      run_on_cpu(inputs[j], &results[j], transitions, starting_state,
                 input_length, output_length);
    }
#else

#if FSM_INPUTS_WITH_OFFSETS //testing FSMs with offsets

#pragma omp parallel for default(none)                                         \
    shared(num_test_cases, inputs_offset, results_offset, offsets, transitions, starting_state,       \
           input_length, output_length) schedule(guided)
    for (int j = 0; j < num_test_cases; j++) {
      int offset = offsets[j];
      run_on_cpu(&inputs_offset[offset], &results_offset[offset], transitions, starting_state,
                 input_length, output_length);
    }
#else

    // testing FSMs without offsets
#pragma omp parallel for default(none)                                         \
    shared(num_test_cases, inputs, results, transitions, starting_state,       \
           input_length, output_length) schedule(guided)
    for (int j = 0; j < num_test_cases; j++) {

      run_on_cpu(inputs[j], results[j], transitions, starting_state,
                 input_length, output_length);
    }

#endif
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

#if BMRK_C || FSM_INPUTS_WITH_OFFSETS
  if (do_print_results)
    compare_results(results, NULL, num_test_cases);

  free(inputs);
  free(results);
#else
  if (do_print_results)
    compare_results_char(results, NULL, num_test_cases);

  for(int i=0; i < num_test_cases; i++) {
    free(inputs[i]);
    free(results[i]);
  }
#endif

}
