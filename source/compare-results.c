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
#include "../kernel-gen/cpu-gen.h"

void compare_results_automotive(struct result * results, struct result * exp_results, int num_test_cases)
{
  for(int i = 0; i < num_test_cases; i++)
  {
    struct result out_result = results[i];
    int test_case_num = out_result.test_case_num;
    
    printf("TC %d: %d\n", test_case_num, out_result.result);
  }
}

void compare_results_eembc_automotive(struct result * results, struct result * exp_results, int num_test_cases)
{
  for(int i = 0; i < num_test_cases; i++)
  {
    printf("TC %d ", results[i].test_case_num);
    for(int j = 0; j < 256; j++)
    {
      printf("%d ", results[i].RAMfile[j]);
    }
    printf("\n");
  }
}

void compare_results_eembc_telecom(struct result * results, struct result * exp_results, int num_test_cases)
{
  for(int i = 0; i < num_test_cases; i++)
  {
    printf("TC %d ", results[i].test_case_num);
    for(int j = 0; j < 32; j++)
    {
      printf("%d ", results[i].result[j]);
    }
    printf("\n");
  }
}
