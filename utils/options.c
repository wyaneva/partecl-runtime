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
#include "options.h"
#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parseYNOption(char **argv, int i, char *arg, int *value) {
  if (strcmp(argv[i + 1], "Y") != 0 && strcmp(argv[i + 1], "N") != 0) {
    printf("Please, provide value Y or N for arg %s.\n", arg);
    return FAIL;
  }

  if (strcmp(argv[i + 1], "Y") == 0)
    *value = 1;
  else
    *value = 0;

  return SUCCESS;
}

// optional arguments
// int* ldim - from gpu code
// int* do_choose_device - from gpu code
// int* num_chunks - from gpu code
int read_options(int argc, char **argv, int *num_test_cases,
                 int *do_compare_outputs, int *do_time, int *num_runs, int *ldim,
                 int *do_choose_device, int *num_chunks) {
  if (argc < 2) {
    printf("Correct usage: gpu-test [number of test cases] (-results Y/N) "
           "(-time Y/N) (-runs ..number..) (-ldim ..number..) (-choose Y/N) "
           "(-chunks ..number..) \n");
    return FAIL;
  }
  *num_test_cases = atoi(argv[1]);

  if (argc > 2) {
    for (int i = 2; i < argc; i += 2) {
      char *label = argv[i];

      if (i == argc - 1) {
        printf("Please, provide a value for arg %s.\n", label);
        return FAIL;
      }

      // OUTPUTS
      if (strcmp(label, "-results") == 0 && do_compare_outputs)
      {
        if (!parseYNOption(argv, i, label, do_compare_outputs))
          return FAIL;
      }

      // TIME
      else if (strcmp(label, "-time") == 0 && do_time) // do time
      {
        if (!parseYNOption(argv, i, label, do_time))
          return FAIL;
      }

      // RUNS
      else if (strcmp(label, "-runs") == 0 && num_runs) // number of runs
      {
        *num_runs = atoi(argv[i + 1]);
      }

      // LDIM
      else if (strcmp(label, "-ldim") == 0 && ldim) // work-group size
      {
        *ldim = atoi(argv[i + 1]);
      }

      // DO_CHOOSE_DEVICE
      else if (strcmp(label, "-choose") == 0 &&
               do_choose_device) // do choose device
      {
        if (!parseYNOption(argv, i, label, do_choose_device))
          return FAIL;
      }

      // OVERLAP
      else if (strcmp(label, "-chunks") == 0 && num_chunks) {
        *num_chunks = atoi(argv[i + 1]);
        if (*num_chunks < 1) {
          printf(
              "Please, provide a number of overlapping input chunks >= 1.\n");
          return FAIL;
        }
      } else {
        printf("Arg %s is not valid.\n", label);
        return FAIL;
      }
    }
  }

  return SUCCESS;
}

