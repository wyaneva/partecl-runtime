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

#ifndef CL_UTILS_H
#define CL_UTILS_H
#include <string.h>
#include <stdio.h>
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include "cl-utils.h"

cl_kernel kernel_from_string(cl_context context, char const *kernel_source, char const *kernel_name, char const *options)
{
  size_t sizes[] = { strlen(kernel_source) };

  cl_int err;
  cl_program program = clCreateProgramWithSource(context, 1, &kernel_source, sizes, &err);
  if(err != CL_SUCCESS)
    printf("error: clCreateProgramWithSource: %d\n", err);

  cl_int build_status = clBuildProgram(program, 0, NULL, options, NULL, NULL);

  if (build_status != CL_SUCCESS)
  {
    //get the device
    cl_device_id device;
    err = clGetProgramInfo(program, CL_PROGRAM_DEVICES, sizeof(device), &device, NULL);
    if(err != CL_SUCCESS)
      printf("error: clGetProgramInfo: %d\n", err);

    //get the device name
    char device_name[20];
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
    if(err != CL_SUCCESS)
      printf("error: clGetDeviceInfo: %d\n", err);

    //get the binaries (for debugging)
    size_t num_binaries;
    err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, 0, NULL, &num_binaries);
    if(err != CL_SUCCESS)
      printf("error: clGetProgramInfo: %d\n", err);

    size_t binary_sizes[num_binaries];
    err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(binary_sizes), binary_sizes, NULL);
    if(err != CL_SUCCESS)
      printf("error: clGetProgramInfo: %d\n", err);

    char* binaries[num_binaries];
    for(int i = 0; i < num_binaries; i++)
      binaries[i] = (char*)malloc(binary_sizes[i]*sizeof(char));
    err = clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(binaries), binaries, NULL);
    if(err != CL_SUCCESS)
      printf("error: clGetProgramInfo: %d\n", err);

    for(int i = 0; i < num_binaries; i++)
      printf("%s\n", binaries[i]);

    //get the build log
    size_t log_size;
    err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    if(err != CL_SUCCESS)
      printf("error: clGetProgramBuildInfo 1: %d\n", err);

    char *log = (char *) malloc(log_size);

    err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
    if(err != CL_SUCCESS)
      printf("error: clGetProgramBuildInfo 2: %d\n", err);

    fprintf(stderr, "*** build of '%s' on '%s' said:\n%s\n*** (end of message)\n",
      kernel_name, device_name, log);

    free(log);
  }

  // fish the kernel out of the program
  cl_kernel kernel = clCreateKernel(program, kernel_name, &err);
  if(err != CL_SUCCESS)
    printf("error: clCreateKernel: %d\n", err);

  clReleaseProgram(program);

  return kernel;
}

void create_context_on_gpu(cl_context *context, cl_command_queue *queue)
{
  cl_int err;

  //get platforms - choose the first platform
  cl_uint platform_count;
  err = clGetPlatformIDs(0, NULL, &platform_count);
  if(err != CL_SUCCESS)
    printf("error: clGetPlatformIDs: %d\n", err);
  cl_platform_id *platforms = (cl_platform_id *)malloc(platform_count*sizeof(cl_platform_id));
  err = clGetPlatformIDs(platform_count, platforms, NULL);
  if(err != CL_SUCCESS)
    printf("error: clGetPlatformIDs: %d\n", err);

  //get devices - choose the first device
  //TODO: Add asking for which device
  cl_uint dev_count;
  err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &dev_count);
  if(err != CL_SUCCESS)
    printf("error: clGetDeviceIDs: %d\n", err);
  cl_device_id *devices = (cl_device_id *)malloc(dev_count*sizeof(cl_device_id));
  err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, dev_count, devices, NULL);
  if(err != CL_SUCCESS)
    printf("error: clGetDeviceIDs: %d\n", err);

  cl_platform_id plat = platforms[0];
  cl_device_id dev = devices[0];

  //print the device info:
  //name
  char buf[100];
  err = clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(buf), buf, NULL);
  if(err != CL_SUCCESS)
    printf("error: clGetDeviceInfo: %d\n", err);
  else
    printf("Device: %s \n", buf);

  //number of compute units
  cl_uint num_compute_units;
  err = clGetDeviceInfo(dev, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(num_compute_units), &num_compute_units, NULL);
  if(err != CL_SUCCESS)
    printf("error: clGetDeviceInfo: %d\n", err);
  else
    printf("Compute units: %d \n", num_compute_units);

  //work-group size
  size_t workgroup_size;
  err = clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(workgroup_size), &workgroup_size, NULL);
  if(err != CL_SUCCESS)
    printf("error: clGetDeviceInfo: %d\n", err);
  else
    printf("Work-group size: %ld \n", workgroup_size);

  //work-group size
  size_t local_memory_size;
  err = clGetDeviceInfo(dev, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_memory_size), &local_memory_size, NULL);
  if(err != CL_SUCCESS)
    printf("error: clGetDeviceInfo: %d\n", err);
  else
    printf("Local memory size (in bytes): %ld \n", local_memory_size);

  char version[20];
  err = clGetDeviceInfo(dev, CL_DEVICE_VERSION, sizeof(version), version, NULL);
  if(err != CL_SUCCESS)
    printf("error: clGetDeviceInfo: %d\n", err);
  else
    printf("Version: %s \n", version);

  free(platforms);
  free(devices);

  // create a context
  cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties) plat, 0};

  *context = clCreateContext(cps, 1, &dev, NULL, NULL, &err);
  if(err != CL_SUCCESS)
    printf("error: clCreateContext: %d\n", err);

  // create a command queue
  cl_command_queue_properties qprops = 0;
  qprops |= CL_QUEUE_PROFILING_ENABLE;

   if (queue)
   {
     *queue = clCreateCommandQueue(*context, dev, qprops, &err);
     if(err != CL_SUCCESS)
       printf("error: clCreateCommandQueue: %d\n", err);
   }
}

#endif
