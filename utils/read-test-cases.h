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

#ifndef READ_TEST_CASES_H
#define READ_TEST_CASES_H

#include "../kernel-gen/structs.h"
#include "utils.h"

int read_test_cases(struct partecl_input *, int, struct aggr *);
int read_test_cases_chunk(char *inputs[], int num_test_cases,
                          int *test_index_total, size_t chunk_size,
                          int test_id_start, struct aggr *aggregate,
                          int *test_id_end);
 
#endif
