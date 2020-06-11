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

#include "read-test-cases.h"
#include "../kernel-gen/cpu-gen.h"
#include "../utils/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TC_FILENAME "../kernel-gen/tests.txt"

int parseString(char **arg, char **bptr) {
  assert(**bptr == '"' && "parseString should start with \".");
  // skip the opening quotes
  (*bptr)++;

  // create current ptr
  char *cptr = *bptr;

  // count the characters we want to copy
  int num_chars = 0;
  while (*cptr != '"') {
    if (*cptr == '\\') {
      char *nextchar = cptr;
      nextchar++;
      if (*nextchar == '\\' || *nextchar == '"') {
        // do not count, skip to the next char
        cptr++;
      }
    }
    num_chars++;
    cptr++;
  }

  // copy the chars to arg
  cptr = *bptr;
  *arg = (char *)malloc(sizeof(char) * (num_chars + 1));
  for (int i = 0; i < num_chars; i++) {
    if (*cptr == '\\') {
      char *nextchar = cptr;
      nextchar++;
      if (*nextchar == '\\' || *nextchar == '"') {
        // skip to next char
        cptr++;
      }
    }
    (*arg)[i] = *cptr;
    cptr++;
  }
  (*arg)[num_chars] = '\0';

  assert(*cptr == '"' && "parseString should end with \".");
  cptr++;
  *bptr = cptr;
  return SUCCESS;
}

int copyToken(char **token, char **bptr) {
  // create current ptr
  char *cptr = *bptr;

  // count the characters we want to copy
  int num_chars = 0;
  while (*cptr != ' ' && *cptr != '\n') {
    num_chars++;
    cptr++;
  }

  // copy the chars to token
  cptr = *bptr;
  *token = (char *)malloc(sizeof(char) * (num_chars + 1));
  for (int i = 0; i < num_chars; i++) {
    (*token)[i] = *cptr;
    cptr++;
  }
  (*token)[num_chars] = '\0';
  *bptr = cptr;

  return SUCCESS;
}

int parseFile(char **arg, char **bptr) {
  assert(**bptr == '<' && "parseFile should start with <.");
  // consume the < char
  (*bptr)++;

  // consume white spaces
  while (**bptr == ' ')
    (*bptr)++;

  // filename or string
  if (**bptr == '\"') {
    // string
    return parseString(arg, bptr);
  } else {
    // find the filename
    char *filename;
    copyToken(&filename, bptr);
    // append ../kernel_gen/ directory to it, as input files will be copied
    // there
    char dirname[] = "../kernel-gen/";
    char *dirandfile = (char *)malloc(sizeof(char *) *
                                      (strlen(dirname) + strlen(filename) + 1));
    *dirandfile = '\0';
    strcat(dirandfile, dirname);
    strcat(dirandfile, filename);
    *arg = read_file(dirandfile);

    free(filename);
    free(dirandfile);
  }

  return SUCCESS;
}

int parseArg(char **arg, char **bptr) {
  // consume starting white spaces
  while (**bptr == ' ')
    (*bptr)++;

  if (**bptr == '\n')
    return SUCCESS;

  // handle string
  if (**bptr == '"') {
    return parseString(arg, bptr);
  }

  // handle test inputs from a file
  if (**bptr == '<') {
    return parseFile(arg, bptr);
  }

  // handle others
  copyToken(arg, bptr);

  return SUCCESS;
}

int read_test_cases(struct partecl_input *inputs, int num_test_cases) {
  int index = 0;
  char line[100000];

  FILE *file = fopen(TC_FILENAME, "r");
  if (file == NULL) {
    printf("Could not find test file %s.\n", TC_FILENAME);
    return FAIL;
  }

  while (index < num_test_cases) {
    if (fgets(line, sizeof(line), file) == NULL) {
      // reopen the file to read the test cases again
      fclose(file);
      file = fopen(TC_FILENAME, "r");
      fgets(line, sizeof(line), file);
    }

    char **values = (char **)malloc(sizeof(char *));
    int num_values = 0;
    char *bptr = &line[0];
    while (*bptr != '\n') {
      char *inputparse;
      int parse = parseArg(&inputparse, &bptr);

      if (parse == SUCCESS) // command-line arg
      {
        num_values++;
        values = (char **)realloc(values, sizeof(char *) * (num_values));
        if (values == NULL) {
          printf(
              "read_test_cases: realloc values: Failed reallocating memory!\n");
          return FAIL;
        }
        values[num_values - 1] = inputparse;
      }
    }

    populate_inputs(&inputs[index], num_values, values);
    index++;

    // free pointers
    for (int i = 0; i < num_values; i++)
      free(values[i]);
    free(values);
  }
  fclose(file);

  return SUCCESS;
}

