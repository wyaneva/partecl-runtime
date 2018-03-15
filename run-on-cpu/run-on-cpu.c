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

#include "../kernel-gen/cpu-gen.h"
#include "../kernel-gen/structs.h"
#include "../kernel-gen/fsm.h"
#include "../utils/options.h"
#include "../utils/read-test-cases.h"
#include "../utils/timing.h"
#include "../utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../source/fsm.c"

//int run_main(struct partecl_input input, struct partecl_result *result);
int run_main(struct partecl_input input, struct partecl_result *result,
             struct transition *transitions, int num_transitions,
             int input_length, int output_length);

int main_cpu(int argc, char **argv, struct transition *transitions,
             int *num_transitions, int *input_length, int *output_length) {
  //if (argc < 3) {
  //  printf("Please provide an input filename and an input sequence.\n");
  //  return 0;
  //}

  // inputs
  char *filename = argv[0];
  //char *input_ptr = argv[2];

  // read the fsm
  // number of states
  *num_transitions = read_parameter(filename, NUM_TRANSITIONS);
  if (*num_transitions == -1) {
    printf("File %s does not specify a number of transitions. Exiting. \n",
           filename);
    return 0;
  }

  // input length
  *input_length = read_parameter(filename, INPUT_LENGTH);
  if (*input_length == -1) {
    printf("File %s does not specify input length. Exiting. \n", filename);
    return 0;
  }

  // output length
  *output_length = read_parameter(filename, OUTPUT_LENGTH);
  if (*output_length == -1) {
    printf("File %s does not specify output length. Exiting. \n", filename);
    return 0;
  }

  // transitions
  size_t size_transitions = sizeof(struct transition) * (*num_transitions);
  transitions =
      (struct transition *)malloc(size_transitions);
  read_fsm(filename, transitions);

 // original code ends here

  return 1;
}

void run_on_cpu(struct partecl_input input, struct partecl_result *result,
                struct transition *transitions, int num_transitions,
                int input_length, int output_length) {

  result->test_case_num = input.test_case_num;
  run_main(input, result, transitions, num_transitions, input_length, output_length);
}

int main(int argc, char **argv) {
  int do_print_results = HANDLE_RESULTS;
  int num_runs = NUM_RUNS;
  int do_time = DO_TIME;
  int num_test_cases = 1;

  if (read_options(argc, argv, &num_test_cases, &do_print_results, &do_time,
                   &num_runs, NULL, NULL, NULL) == FAIL)
    return 0;
  printf("Device: CPU.\n");
  printf("Number of test cases %d.\n", num_test_cases);
#pragma omp parallel default(none)
  {
    int tid = omp_get_thread_num();
    if (tid == 0)
      printf("Number of threads = %d\n", omp_get_num_threads());
  }
  if (do_time)
    printf("Time in ms\n");

  struct partecl_input *inputs;
  size_t inputs_size = sizeof(struct partecl_input) * num_test_cases;
  inputs = (struct partecl_input *)malloc(inputs_size);
  struct partecl_result *results;
  size_t results_size = sizeof(struct partecl_result) * num_test_cases;
  results = (struct partecl_result *)malloc(results_size);

  // read the test cases
  if (read_test_cases(inputs, num_test_cases) == FAIL)
    return 0;

  // read the fsm
  char *filename = "train4.kiss2";
  int num_transitions;
  int input_length;
  int output_length;
  struct transition *transitions;
  main_cpu(2, &filename, transitions, &num_transitions, &input_length, &output_length);

  for (int i = 0; i < num_runs; i++) {
    struct timespec time1, time2;
    get_timestamp(&time1);

#pragma omp parallel for default(none) shared(num_test_cases, inputs, results) \
    schedule(static)
    for (int j = 0; j < num_test_cases; j++) {
      run_on_cpu(inputs[j], &results[j]);
    }

    get_timestamp(&time2);
    double time_in_secs = timestamp_diff_in_seconds(time1, time2);
    double time_cpu = time_in_secs * 1000;

    if (do_time)
      printf("%f \n", time_cpu);
  }

  if (do_print_results)
    compare_results(results, NULL, num_test_cases);

  free(inputs);
  free(results);
}
