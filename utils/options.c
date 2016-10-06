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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "options.h"
#include "utils.h"

int read_options(int argc, char **argv, int* num_test_cases, int* handle_results, int* do_time, int* num_runs)
{  
  
  if(argc < 2)
  {
    printf("Correct usage: test-on-gpu [number of test cases] (-results Y/N) (-time Y/N) (-runs ..number..)\n");
    return FAIL;
  }
  *num_test_cases = atoi(argv[1]);

  if(argc > 2)
  {
    for(int i = 2; i < argc-1; i+=2)
    {
      char* label = argv[i];
      if(strcmp(label, "-results") == 0) //compare results
      {
        if(strcmp(argv[i+1],"Y") != 0 && strcmp(argv[i+1],"N") != 0)
        {
          printf("Please, provide value Y or N for arg -c.\n");
          return FAIL;
        }
        
        if(strcmp(argv[i+1],"Y") == 0)
          *handle_results = 1;

        if(strcmp(argv[i+1],"N") == 0)
          *handle_results = 0;
      }
      else if(strcmp(label, "-runs") == 0) //number of runs
      {
        *num_runs = atoi(argv[i+1]);
      }
      else if(strcmp(label, "-time") == 0) //do time
      {
        if(strcmp(argv[i+1],"Y") != 0 && strcmp(argv[i+1],"N") != 0)
        {
          printf("Please, provide value Y or N for arg -t.\n");
          return FAIL;
        }
        
        if(strcmp(argv[i+1],"Y") == 0)
          *do_time = 1;

        if(strcmp(argv[i+1],"N") == 0)
          *do_time = 0;
      }
      else
      {
        printf("Arg %s is not valid.\n", label);
        return FAIL;
      }
    }
  }
  
  return SUCCESS;
}

