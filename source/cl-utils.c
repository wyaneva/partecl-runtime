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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "cl-utils.h"
#include <CL/cl.h>

void choose_device(cl_platform_id *rplatform, cl_device_id *rdevice,
                   bool do_choose) {
  cl_int err;

  // get the platforms
  cl_uint platform_count;
  err = clGetPlatformIDs(0, NULL, &platform_count);
  if (err != CL_SUCCESS)
    printf("error: clGetPlatformIDs (platform_count): %d\n", err);
  cl_platform_id *platforms =
      (cl_platform_id *)malloc(platform_count * sizeof(cl_platform_id));
  err = clGetPlatformIDs(platform_count, platforms, NULL);
  if (err != CL_SUCCESS)
    printf("error: clGetPlatformIDs (platforms): %d\n", err);

  int platformidx;
  if (!do_choose) {
    platformidx = 0;
  } else {
    // prompt user
    printf("Please choose a platform: \n");
    for (int i = 0; i < (int)platform_count; i++) {
      cl_platform_id platform = platforms[i];
      char buf[100];
      err =
          clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(buf), buf, NULL);
      if (err != CL_SUCCESS)
        printf("error: clGetPlatformInfo (CL_PLATFORM_NAME): %d\n", err);
      else
        printf("[%d]: %s\n", i, buf);
    }
    scanf("%d", &platformidx);
    if (platformidx < 0 || platformidx >= (int)platform_count) {
      printf("%d is not a valid platform. Defaulting to platform 0.\n",
             platformidx);
      platformidx = 0;
    }
  }
  *rplatform = platforms[platformidx];

  // get the devices
  cl_uint dev_count;
  err = clGetDeviceIDs(*rplatform, CL_DEVICE_TYPE_ALL, 0, NULL, &dev_count);
  if (err != CL_SUCCESS)
    printf("error: clGetDeviceIDs (dev_count): %d\n", err);
  cl_device_id *devices =
      (cl_device_id *)malloc(dev_count * sizeof(cl_device_id));
  err =
      clGetDeviceIDs(*rplatform, CL_DEVICE_TYPE_ALL, dev_count, devices, NULL);
  if (err != CL_SUCCESS)
    printf("error: clGetDeviceIDs (devices): %d\n", err);

  int deviceidx;
  if (!do_choose) {
    deviceidx = 0;
  } else {
    // prompt the user
    printf("Please choose a device: \n");
    for (int i = 0; i < (int)dev_count; i++) {
      cl_device_id dev = devices[i];

      char buf[100];
      err = clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(buf), buf, NULL);
      if (err != CL_SUCCESS)
        printf("error: clGetDeviceInfo (CL_DEVICE_NAME): %d\n", err);
      else
        printf("[%d]: %s \n", i, buf);
    }
    scanf("%d", &deviceidx);
    if (deviceidx < 0 || deviceidx >= (int)dev_count) {
      printf("%d is not a valid device index. Defaulting to platform 0.\n",
             deviceidx);
      deviceidx = 0;
    }
  }
  *rdevice = devices[deviceidx];

  free(platforms);
  free(devices);
}

cl_kernel kernel_from_string(cl_context context, char const *kernel_source,
                             char const *kernel_name, char const *options) {
  size_t sizes[] = {strlen(kernel_source)};

  cl_int err;
  cl_program program =
      clCreateProgramWithSource(context, 1, &kernel_source, sizes, &err);
  if (err != CL_SUCCESS)
    printf("error: clCreateProgramWithSource: %d\n", err);

  cl_int build_status = clBuildProgram(program, 0, NULL, options, NULL, NULL);

  if (build_status != CL_SUCCESS) {
    // get the device
    cl_device_id device;
    err = clGetProgramInfo(program, CL_PROGRAM_DEVICES, sizeof(device), &device,
                           NULL);
    if (err != CL_SUCCESS)
      printf("error: clGetProgramInfo (CL_PROGRAM_DEVICES): %d\n", err);

    // get the binaries (for debugging)
    size_t num_binaries;
    err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, 0, NULL,
                           &num_binaries);
    if (err != CL_SUCCESS)
      printf("error: clGetProgramInfo (num_binaries): %d\n", err);

    size_t binary_sizes[num_binaries];
    err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES,
                           sizeof(binary_sizes), binary_sizes, NULL);
    if (err != CL_SUCCESS)
      printf("error: clGetProgramInfo (binary_sizes): %d\n", err);

    char *binaries[num_binaries];
    for (size_t i = 0; i < num_binaries; i++)
      binaries[i] = (char *)malloc(binary_sizes[i] * sizeof(char));
    err = clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(binaries),
                           binaries, NULL);
    if (err != CL_SUCCESS)
      printf("error: clGetProgramInfo (binaries): %d\n", err);

    for (size_t i = 0; i < num_binaries; i++)
      printf("%s\n", binaries[i]);

    // get the build log
    size_t log_size;
    err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL,
                                &log_size);
    if (err != CL_SUCCESS)
      printf("error: clGetProgramBuildInfo (log_size): %d\n", err);

    char *log = (char *)malloc(log_size);

    err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size,
                                log, NULL);
    if (err != CL_SUCCESS)
      printf("error: clGetProgramBuildInfo (log): %d\n", err);

    fprintf(stderr, "*** build of '%s' said:\n%s\n*** (end of message)\n",
            kernel_name, log);

    free(log);
  }

  // fish the kernel out of the program
  cl_kernel kernel = clCreateKernel(program, kernel_name, &err);
  if (err != CL_SUCCESS)
    printf("error: clCreateKernel: %d\n", err);

  clReleaseProgram(program);

  return kernel;
}

void create_context_on_gpu(cl_context *context, cl_device_id *dev,
                           bool do_choose_device) {
  cl_int err;

  cl_platform_id plat;
  choose_device(&plat, dev, do_choose_device);

  // print the device info:
  // name
  char buf[100];
  err = clGetDeviceInfo(*dev, CL_DEVICE_NAME, sizeof(buf), buf, NULL);
  if (err != CL_SUCCESS)
    printf("error: clGetDeviceInfo (CL_DEVICE_NAME): %d\n", err);
  else
    printf("Device: %s \n", buf);

  // number of compute units
  cl_uint num_compute_units;
  err = clGetDeviceInfo(*dev, CL_DEVICE_MAX_COMPUTE_UNITS,
                        sizeof(num_compute_units), &num_compute_units, NULL);
  if (err != CL_SUCCESS)
    printf("error: clGetDeviceInfo (CL_DEVICE_MAX_COMPUTE_UNITS): %d\n", err);
  else
    printf("Compute units: %d \n", num_compute_units);

  // work-group size
  size_t workgroup_size;
  err = clGetDeviceInfo(*dev, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                        sizeof(workgroup_size), &workgroup_size, NULL);
  if (err != CL_SUCCESS)
    printf("error: clGetDeviceInfo (CL_DEVICE_MAX_WORK_GROUP_SIZE): %d\n", err);
  else
    printf("Work-group size: %ld \n", workgroup_size);

  // local mem size
  size_t local_memory_size;
  err = clGetDeviceInfo(*dev, CL_DEVICE_LOCAL_MEM_SIZE,
                        sizeof(local_memory_size), &local_memory_size, NULL);
  if (err != CL_SUCCESS)
    printf("error: clGetDeviceInfo (CL_DEVICE_LOCAL_MEM_SIZE): %d\n", err);
  else
    printf("Local memory size (in bytes): %ld \n", local_memory_size);

  // global mem size
  size_t global_memory_size;
  err = clGetDeviceInfo(*dev, CL_DEVICE_GLOBAL_MEM_SIZE,
                        sizeof(global_memory_size), &global_memory_size, NULL);
  if (err != CL_SUCCESS)
    printf("error: clGetDeviceInfo (CL_DEVICE_GLOBAL_MEM_SIZE): %d\n", err);
  else
    printf("Global memory size (in bytes): %ld \n", global_memory_size);


  char version[100];
  err =
      clGetDeviceInfo(*dev, CL_DEVICE_VERSION, sizeof(version), version, NULL);
  if (err != CL_SUCCESS)
    printf("error: clGetDeviceInfo (CL_DEVICE_VERSION): %d\n", err);
  else
    printf("Version: %s \n", version);

  // create a context
  cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM,
                                  (cl_context_properties)plat, 0};

  *context = clCreateContext(cps, 1, dev, NULL, NULL, &err);
  if (err != CL_SUCCESS)
    printf("error: clCreateContext: %d\n", err);
}

void create_command_queue(cl_command_queue *queue, cl_context *context,
                          cl_device_id *dev) {
  cl_int err;

  // create a command queue
  cl_command_queue_properties qprops = 0;
  qprops |= CL_QUEUE_PROFILING_ENABLE;
  qprops |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;

  if (queue) {
    *queue = clCreateCommandQueue(*context, *dev, qprops, &err);
    if (err != CL_SUCCESS)
      printf("error: clCreateCommandQueue: %d\n", err);
  }
}

#endif
