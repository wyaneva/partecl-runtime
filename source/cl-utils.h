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
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <stdbool.h>

void choose_device(cl_platform_id *, cl_device_id *, bool);
void create_context_on_gpu(cl_context *, cl_device_id *, bool);
void create_command_queue(cl_command_queue *, cl_context *, cl_device_id *);
size_t get_constant_mem_size(cl_device_id *);
size_t get_local_mem_size(cl_device_id *);
size_t get_max_mem_alloc_size(cl_device_id *);
cl_bool get_gpu_ovelap_nv(cl_device_id *);
cl_kernel kernel_from_string(cl_context, char const *, char const *,
                             char const *);

#endif
