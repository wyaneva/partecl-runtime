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
#include "cl-utils.h"
//#include "compare-results.h"

#define GPU_SOURCE "../kernel-gen/test.cl"
#define KERNEL_NAME "main_kernel"

void calculate_dimensions(size_t[3], size_t[3], int);
void read_expected_results(struct partecl_result *, int);

int main(int argc, char **argv)
{
  double trans_inputs = 0.0;
  double trans_results = 0.0;
  double time_gpu = 0.0;
  double end_to_end = 0.0;
  struct timespec ete_start, ete_end;
  cl_ulong ev_start_time, ev_end_time;

  int do_compare_results = HANDLE_RESULTS;
  int num_runs = NUM_RUNS;
  int do_time = DO_TIME;
  int num_test_cases = 1;

  if(read_options(argc, argv, &num_test_cases, &do_compare_results, &do_time, &num_runs) == FAIL)
    return 0;

  //allocate CPU memory and generate test cases
  struct partecl_input * inputs;
  size_t size = sizeof(struct partecl_input) * num_test_cases;
  inputs = (struct partecl_input*)malloc(size);
  struct partecl_result * results;
  size_t size_results = sizeof(struct partecl_result) * num_test_cases;
  results = (struct partecl_result *)malloc(size_results);

  //create queue and context
  cl_context ctx;
  cl_command_queue queue;
  cl_int err;
  create_context_on_gpu(&ctx, &queue);

  //allocate cpu and gpu memory for inputs
  /*
  cl_mem buf_inputs = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, size, inputs, &err);
  if(err != CL_SUCCESS)
    printf("error: clCreateBuffer: %d\n", err);
  
  //map the cpu memory to the gpu to generate the test cases
  void* p_map_inputs = clEnqueueMapBuffer(queue, buf_inputs, CL_TRUE, CL_MAP_WRITE, 0, 0, 0, NULL, NULL, &err);
  if(err != CL_SUCCESS)
    printf("error: clEnqueueMapBuffer: %d\n", err);
    */

  //read the test cases
  if(read_test_cases(inputs, num_test_cases) == FAIL)
    return 0;

  //unmap inputs from gpu memory
  /*
  err = clEnqueueUnmapMemObject(queue, buf_inputs, p_map_inputs, 0, NULL, NULL);
  if(err != CL_SUCCESS)
    printf("error: clEnqueueUnmapMemObject: %d\n", err);
    */

  //create kernel
  char *knl_text = read_file(GPU_SOURCE);
  //add include directory for the kernel header files (structs & clClibc)

  //char options[] = "-I /home/vanya/partecl-runtime/kernel-gen/ -I /home/vanya/clclibc/";
  char options[] = "-I /afs/inf.ed.ac.uk/user/s08/s0835905/partecl-runtime/kernel-gen/ -I /afs/inf.ed.ac.uk/user/s08/s0835905/clClibc/";
  cl_kernel knl = kernel_from_string(ctx, knl_text, KERNEL_NAME, options);
  free(knl_text);

  struct partecl_result * exp_results;
  exp_results = (struct partecl_result *)malloc(sizeof(struct partecl_result)*num_test_cases);
  if(do_compare_results)
    read_expected_results(exp_results, num_test_cases);

  if(do_time)
  {
    printf("Number of test cases: %d\n", num_test_cases);
    printf("Time in ms\n");
    printf("trans-inputs trans-results exec-kernel time-total \n");
  }

  for(int i=0; i < num_runs; i++)
  {
   //allocate device memory
    cl_mem buf_inputs = clCreateBuffer(ctx, CL_MEM_READ_WRITE, size, NULL, &err);
    if(err != CL_SUCCESS)
      printf("error: clCreateBuffer: %d\n", err);

    cl_mem buf_results = clCreateBuffer(ctx, CL_MEM_READ_WRITE, size_results, NULL, &err);
    if(err != CL_SUCCESS)
      printf("error: clCreateBuffer: %d\n", err);

    //clalculate dimensions
    size_t gdim[3], ldim[3];
    calculate_dimensions(gdim, ldim, num_test_cases);

    get_timestamp(&ete_start);

    //transfer input to device
    cl_event event_inputs;
    err = clEnqueueWriteBuffer(queue, buf_inputs, CL_FALSE, 0, size, inputs, 0, NULL, &event_inputs);
    if(err != CL_SUCCESS)
      printf("error: clEnqueueWriteBuffer: %d\n", err);
    
    //add kernel arguments
    err = clSetKernelArg(knl, 0, sizeof(cl_mem), &buf_inputs);
    if(err != CL_SUCCESS)
      printf("error: clSetKernelArg 0: %d\n", err);

    err = clSetKernelArg(knl, 1, sizeof(cl_mem), &buf_results);
    if(err != CL_SUCCESS)
      printf("error: clSetKernelArg 1: %d\n", err);

    //launch kernel
    cl_event event_kernel = 0;
    err = clEnqueueNDRangeKernel(queue, knl, 1, NULL, gdim, ldim, 0, NULL, &event_kernel);
    //err = clEnqueueNDRangeKernel(queue, knl, 1, NULL, gdim, ldim, 0, NULL, NULL);
    if(err != CL_SUCCESS)
      printf("error: clEnqueueNDRangeKernel: %d\n", err);

    //transfer results back
    cl_event event_results;
    err = clEnqueueReadBuffer(queue, buf_results, CL_FALSE, 0, size_results, results, 0, NULL, &event_results);
    if(err != CL_SUCCESS)
      printf("error: clEnqueueReadBuffer: %d\n", err);

    err = clFinish(queue);
    if(err != CL_SUCCESS)
      printf("error: clFinish: %d\n", err);

    get_timestamp(&ete_end);

    //free memory buffers
    err = clReleaseMemObject(buf_inputs);
    if(err != CL_SUCCESS)
      printf("error: clReleaseMemObject: %d\n", err);

    err = clReleaseMemObject(buf_results);
    if(err != CL_SUCCESS)
      printf("error: clReleaseMemObjec: %d\n", err);
    
    //gather performance data
    clGetEventProfilingInfo(event_inputs, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ev_start_time, NULL);
    clGetEventProfilingInfo(event_inputs, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ev_end_time, NULL);
    trans_inputs = (double)(ev_end_time - ev_start_time)/1000000;
    clGetEventProfilingInfo(event_results, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ev_start_time, NULL);
    clGetEventProfilingInfo(event_results, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ev_end_time, NULL);
    trans_results = (double)(ev_end_time - ev_start_time)/1000000;

    clGetEventProfilingInfo(event_kernel, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ev_start_time, NULL);
    clGetEventProfilingInfo(event_kernel, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ev_end_time, NULL);
    time_gpu = (double)(ev_end_time - ev_start_time)/1000000;

    end_to_end = timestamp_diff_in_seconds(ete_start, ete_end) * 1000; //in ms
    if(do_time)
      printf("%f %f %f %f \n", trans_inputs, trans_results, time_gpu, end_to_end);
 
    //check results
    if(do_compare_results)
      compare_results(results, exp_results, num_test_cases);

    err = clReleaseEvent(event_inputs);
    if(err != CL_SUCCESS)
      printf("error: clReleaseEvent (event_inputs): %d\n", err);
    err = clReleaseEvent(event_results);
    if(err != CL_SUCCESS)
      printf("error: clReleaseEvent (event_results): %d\n", err);
    err = clReleaseEvent(event_kernel);
    if(err != CL_SUCCESS)
      printf("error: clReleaseEvent (event_kernel): %d\n", err);
  }

  free(inputs);
  free(results);
  free(exp_results);
}

void calculate_dimensions(size_t gdim[3], size_t ldim[3], int num_test_cases)
{
  //calculate local dimension
  size_t ldim0 = num_test_cases;
  int div = num_test_cases/ 999;
  if(div > 0)
    ldim0 = num_test_cases/ (div+1);

  //ensure that the dimensions will be properly distributed across 
  while((num_test_cases / ldim0) * ldim0 != num_test_cases)
  {
    div++;
    if(div > 0)
      ldim0 = num_test_cases / div;
  }

  gdim[0] = num_test_cases;
  gdim[1] = 1;
  gdim[2] = 1;
  ldim[0] = ldim0;
  ldim[1] = 1;
  ldim[2] = 1;
}

/*
void run_on_cpu(input *inputs, int num_test_cases, double *time_cpu)
{
  timestamp_type time1, time2;
  get_timestamp(&time1);
  for(int i = 0; i < num_test_cases; i++)
  {
    struct input input = inputs[i];
    int argc = input.argc;
    char *argv[3];

    argv[1] = (char *)malloc(sizeof(char)*strlen(input.pat_in));
    sprintf(argv[1], "%s", input.pat_in);

    argv[2] = (char *)malloc(sizeof(char)*strlen(input.sub_in));
    sprintf(argv[2], "%s", input.sub_in);

    //change the name of the 'main' function in the original code
    run_main(argc, argv, input.stdin1);
  }
  get_timestamp(&time2);
  double time_in_secs = timestamp_diff_in_seconds(time1, time2);
  *time_cpu = time_in_secs*1000;
}
*/

void read_expected_results(struct partecl_result *results, int num_test_cases)
{
  //TODO:
}

//replace
/*
void compare_results(result * results, result * exp_results, int num_test_cases)
{
  int fail = 0;
  for(int i = 0; i < num_test_cases; i++)
  {
    result out_result = results[i];
    int test_case_num = out_result.test_case_num;
    
    //read expected result
    char exp_file_name[] = "../kernel-gen/outputs/t"; 
    char t_str[5];
    sprintf(t_str, "%d", test_case_num);
    char* filename = (char*)malloc(sizeof(char)*(strlen(exp_file_name)+strlen(t_str)+1));
    *filename = '\0';
    strcat(filename, exp_file_name);
    strcat(filename, t_str);
    char* exp_result = read_file(filename);
    free(filename);

    if(strcmp(out_result.result, exp_result) != 0)
    {
      printf("FAIL! TC %d: \n", test_case_num);
      printf("  Expected: \n%s \n", exp_result);
      printf("  Got: \n%s \n", out_result.result);
      fail++;
    }
    free(exp_result);
  }

  if(fail == 0)
    printf("ALL PASS!\n");
  else
    printf("Failed: %d tests.\n", fail);
}
*/

//tcas
/*
void compare_results(result * results, result * exp_results, int num_test_cases)
{
  int fail = 0;
  for(int i = 0; i < num_test_cases; i++)
  {
    result out_result = results[i];
    int test_case_num = out_result.test_case_num;
    
    //read expected result
    char exp_file_name[] = "../kernel-gen/outputs/t"; 
    char t_str[5];
    sprintf(t_str, "%d", test_case_num);
    char* filename = (char*)malloc(sizeof(char)*(strlen(exp_file_name)+strlen(t_str)+1));
    *filename = '\0';
    strcat(filename, exp_file_name);
    strcat(filename, t_str);
    char* exp_result_str = read_file(filename);
    if(!isdigit(*exp_result_str))
      continue;

    int exp_result = strtol(exp_result_str, NULL, 10);

    if(out_result.result != exp_result)
    {
      printf("FAIL! TC %d: \n", test_case_num);
      printf("  Expected: \n%d \n", exp_result);
      printf("  Got: \n%d \n", out_result.result);
      fail++;
    }
    free(exp_result_str);
    free(filename);
  }

  if(fail == 0)
    printf("ALL PASS!\n");
  else
    printf("Failed: %d tests.\n", fail);
}
*/

//automotive
/*
void compare_results(result * results, result * exp_results, int num_test_cases)
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
*/

//telecom
/*
void compare_results(result * results, result * exp_results, int num_test_cases)
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
*/

/*
//empty
void compare_results(result * results, result * exp_results, int num_test_cases)
{
  //do nothing
}
*/
