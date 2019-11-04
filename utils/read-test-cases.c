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
#include "../source/constants.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TC_FILENAME "../kernel-gen/tests.txt"
#define NUMBER_OF_OCTAL_VALUE_CHARACTERS 3

int copyFilename(char **token, char **bptr) {
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

int parseStdin(char **arg, char **bptr) {
  assert(**bptr == '<' && "parseStdin should start with <.");
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
    copyFilename(&filename, bptr);
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

// read a single chunk of size
// IN: `inputs` - populate with read tests
//     `num_test_cases` - total number of tests to read
//     `chunk_size` - size of chunk in Bytes
//     `test_id_start` - where the chunk starts
// OUT: `aggregate` - average length of total tests read
//      `test_id_end` - where the chunk ends
// both: `test_index_total` - how many tests have we read so far in total
int read_test_cases_chunk(char *inputs[], int num_test_cases,
                          int *test_index_total, size_t chunk_size,
                          int test_id_start, struct aggr *aggregate,
                          int *test_id_end) {
  int test_index_chunk=0;
  int line_index = -1;
  char line[100000];

  FILE *file = fopen(TC_FILENAME, "r");
  if (file == NULL) {
    printf("Could not find test file %s.\n", TC_FILENAME);
    return FAIL;
  }

  //find out how many tests there are in the file
  int total_num_tests = 0;
  while (fgets(line, sizeof(line), file) != NULL) {
    total_num_tests++;
  }
  fclose(file);

  // open the file again to read them
  size_t current_chunk_size = 0;
  int padded_test_lenght = 0;
  file = fopen(TC_FILENAME, "r");
  while (*test_index_total < num_test_cases) {
    if (fgets(line, sizeof(line), file) == NULL) {
      // reopen the file to read the test cases again
      fclose(file);
      file = fopen(TC_FILENAME, "r");
      fgets(line, sizeof(line), file);
    }

    line_index++;
    if (!is_test_chosen(total_num_tests, num_test_cases)) {
      continue;
    }

    char **args = (char **)malloc(sizeof(char *));
    int argc = 0;
    char **stdins = (char **)malloc(sizeof(char *));
    int stdinc = 0;
    char *bptr = &line[0];
    while (*bptr != '\n') {
      char *argparse;
      int parse = parseArg(&argparse, &bptr);

      if (parse == PARSED_ARGV) // command-line arg
      {
        argc++;
        args = (char **)realloc(args, sizeof(char *) * (argc));
        if (args == NULL) {
          printf("realloc args: Failed reallocating memory!\n");
          return FAIL;
        }
        args[argc - 1] = argparse;
      } else if (parse == PARSED_STDIN) // stdin
      {
        // we only support one var for stdin for now
        stdinc++;
        stdins = (char **)realloc(stdins, sizeof(char *) * stdinc);
        if (stdins == NULL) {
          printf("realloc stdins: Failed reallocating memory!\n");
          return FAIL;
        }
        stdins[stdinc - 1] = argparse;
      }
    }

    int test_length = calculate_test_length(args[argc-1]);
    inputs[test_index_chunk] = (char *)malloc(test_length * sizeof(char*));
    strcpy(inputs[test_index_chunk], args[argc-1]);
    update_aggr(aggregate, test_length);

    // TODO
    //printf("%s\n", args[argc-1]);

    // calculate size of current chunk and exit if reached
    if (chunk_size > 0) {
      padded_test_lenght =
          padded_test_lenght >= test_length ? padded_test_lenght : test_length;
      size_t current_chunk_size =
          sizeof(char) * test_index_chunk * padded_test_lenght;
      if (current_chunk_size >= chunk_size) {
        // we have reached our chunksize, so stop reading
        *test_id_end = line_index + 1;
        break;
      }
    }

    // free pointers
    for (int i = 0; i < argc; i++)
      free(args[i]);
    free(args);

    if (stdinc > 0)
      free(stdins[0]);
    free(stdins);
    (*test_index_total)++;
    test_index_chunk++;
  }
  fclose(file);

  return SUCCESS;
}

int read_test_cases(struct partecl_input *inputs, int num_test_cases,
                    struct aggr *aggregate) {
  int test_index = 0;
  int line_index = -1;
  char line[100000];

  FILE *file = fopen(TC_FILENAME, "r");
  if (file == NULL) {
    printf("Could not find test file %s.\n", TC_FILENAME);
    return FAIL;
  }

  //find out how many tests there are in the file
  int total_num_tests = 0;
  while (fgets(line, sizeof(line), file) != NULL) {
    total_num_tests++;
  }
  fclose(file);

  // open the file again to read them
  file = fopen(TC_FILENAME, "r");
  while (test_index < num_test_cases) {
    if (fgets(line, sizeof(line), file) == NULL) {
      // reopen the file to read the test cases again
      fclose(file);
      file = fopen(TC_FILENAME, "r");
      fgets(line, sizeof(line), file);
    }

    line_index++;
    if (!is_test_chosen(total_num_tests, num_test_cases)) {
      continue;
    }

    char **args = (char **)malloc(sizeof(char *));
    int argc = 0;
    char **stdins = (char **)malloc(sizeof(char *));
    int stdinc = 0;
    char *bptr = &line[0];
    while (*bptr != '\n') {
      char *argparse;
      int parse = parseArg(&argparse, &bptr);

      if (parse == PARSED_ARGV) // command-line arg
      {
        argc++;
        args = (char **)realloc(args, sizeof(char *) * (argc));
        if (args == NULL) {
          printf("realloc args: Failed reallocating memory!\n");
          return FAIL;
        }
        args[argc - 1] = argparse;
      } else if (parse == PARSED_STDIN) // stdin
      {
        // we only support one var for stdin for now
        stdinc++;
        stdins = (char **)realloc(stdins, sizeof(char *) * stdinc);
        if (stdins == NULL) {
          printf("realloc stdins: Failed reallocating memory!\n");
          return FAIL;
        }
        stdins[stdinc - 1] = argparse;
      }
    }

    populate_inputs(&inputs[test_index], argc, args, stdinc, stdins);
    int test_length = calculate_test_length(args[argc-1]);
    update_aggr(aggregate, test_length);

    // TODO
    printf("%s\n", args[argc-1]);

    // free pointers
    for (int i = 0; i < argc; i++)
      free(args[i]);
    free(args);

    if (stdinc > 0)
      free(stdins[0]);
    free(stdins);
    test_index++;

  }
  fclose(file);

  return SUCCESS;
}

