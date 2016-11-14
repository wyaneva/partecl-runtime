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
#include <string.h>
#include "../utils/timing.h"
#include "../utils/read-test-cases.h"
#include "../utils/utils.h"
#include "../utils/options.h"
#include "../kernel-gen/structs.h"
#include "../kernel-gen/cpu-gen.h"

//int run_main(int, char**, char*);
int run_main(struct input input, struct result *result);

//telecom
/*
void run_on_cpu(struct input input, struct result *result)
{
  run_main(input, result);
}
void print_results(struct result *result)
{
  printf("TC %d ", (*result).test_case_num);

  for(int i = 0; i < 32; i++)
    printf("%d ", (*result).result[i]);

  printf("\n");
}
*/

//automotive
void run_on_cpu(struct input input, struct result *result)
{
  run_main(input, result);
}
void print_results(struct result *result)
{
  printf("TC %d ", (*result).test_case_num);

  for(int i = 0; i < 256; i++)
    printf("%d ", (*result).RAMfile[i]);

  printf("\n");
}

//replace
/*
void run_on_cpu(struct input input)
{    
  int argc = input.argc;
  char *argv[3];

  argv[1] = (char *)malloc(sizeof(char)*(strlen(input.pat_in)+1));
  sprintf(argv[1], "%s", input.pat_in);

  argv[2] = (char *)malloc(sizeof(char)*(strlen(input.sub_in)+1));
  sprintf(argv[2], "%s", input.sub_in);

  //change the name of the 'main' function in the original code
  run_main(argc, argv, input.stdin1);
}
*/

//tcas 
/*
void run_on_cpu(struct input input)
{
   int argc = input.argc;
   char *argv[13];

   argv[1] = (char *)malloc(sizeof(char)*10);
   sprintf(argv[1], "%d", input.Cur_Vertical_Sep);

   argv[2] = (char *)malloc(sizeof(char)*10);
   sprintf(argv[2], "%d", input.High_Confidence);

   argv[3] = (char *)malloc(sizeof(char)*10);
   sprintf(argv[3], "%d", input.Two_of_Three_Reports_Valid);

   argv[4] = (char *)malloc(sizeof(char)*10);
   sprintf(argv[4], "%d", input.Own_Tracked_Alt);

   argv[5] = (char *)malloc(sizeof(char)*10);
   sprintf(argv[5], "%d", input.Own_Tracked_Alt_Rate);

   argv[6] = (char *)malloc(sizeof(char)*10);
   sprintf(argv[6], "%d", input.Other_Tracked_Alt);

   argv[7] = (char *)malloc(sizeof(char)*10);
   sprintf(argv[7], "%d", input.Alt_Layer_Value);

   argv[8] = (char *)malloc(sizeof(char)*10);
   sprintf(argv[8], "%d", input.Up_Separation);

   argv[9] = (char *)malloc(sizeof(char)*10);
   sprintf(argv[9], "%d", input.Down_Separation);

   argv[10] = (char *)malloc(sizeof(char)*10);
   sprintf(argv[10], "%d", input.Other_RAC);

   argv[11] = (char *)malloc(sizeof(char)*10);
   sprintf(argv[11], "%d", input.Other_Capability);

   argv[12] = (char *)malloc(sizeof(char)*10);
   sprintf(argv[12], "%d", input.Climb_Inhibit);

   //change the name of the 'main' function in the original code
   run_main(argc, argv, NULL);
}
*/

int main(int argc, char** argv)
{
  int do_print_results = HANDLE_RESULTS;
  int num_runs = NUM_RUNS;
  int do_time = DO_TIME;
  int num_test_cases = 1;

  if(read_options(argc, argv, &num_test_cases, &do_print_results, &do_time, &num_runs) == FAIL)
    return 0;
  printf("Device: CPU.\n", num_test_cases);
  printf("Number of test cases %d.\n", num_test_cases);
  if(do_time)
    printf("Time in ms\n");

  struct input * inputs;
  size_t size = sizeof(struct input) * num_test_cases;
  inputs = (struct input *)malloc(size);
  struct result result;

   //read the test cases
  if(read_test_cases(inputs, num_test_cases) == FAIL)
    return 0;

  for(int i = 0; i < num_runs; i++)
  {
    struct timespec time1, time2;
    get_timestamp(&time1);
    for(int j = 0; j < num_test_cases; j++)
    {
      struct input input = inputs[j];

      run_on_cpu(input, &result);

      if(do_print_results && i == 0) //print them only once
        print_results(&result);
    }
    get_timestamp(&time2);
    double time_in_secs = timestamp_diff_in_seconds(time1, time2);
    double time_cpu = time_in_secs*1000;

    if(do_time)
      printf("%f \n", time_cpu);
  }
}
