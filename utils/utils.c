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

char *read_file(const char *filename)
{
  FILE *f = fopen(filename, "r");
  if(!f)
  {
    printf("Reading file %s \n", filename);
    perror(" ERROR");
  }

  // figure out file size
  fseek(f, 0, SEEK_END);
  size_t size = ftell(f);

  fseek(f, 0, SEEK_SET);

  // allocate memory, slurp in entire file
  char *result = (char *) malloc(size+1);
  fread(result, 1, size, f);

  // close, return
  fclose(f);
  result[size] = '\0';

  return result;
}

