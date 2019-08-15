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

void copyToken(char **token, char **bptr) {
  // create current ptr
  char *cptr = *bptr;

  // count the characters we want to copy
  int num_chars = 0;
  while (*cptr != ' ' && *cptr != '\n') {
    num_chars++;
    cptr++;
  }
  cptr = *bptr;
  *token = (char *)malloc(sizeof(char) * (num_chars + 1));

  int char_position = 0;
  while (*cptr != '\n' && *cptr != ' ') {
    // non-printable characters are encoded as \[0-7][0-7][0-7] backslash and 3
    // octal digits.
    if (*cptr == '\\') {
      cptr++;
      if (*cptr == 'a' || *cptr == 'b' || *cptr == 't' || *cptr == 'n' ||
          *cptr == 'v' || *cptr == 'f' || *cptr == 'r' || *cptr == '\\') {
        switch (*cptr) {
        case 'a':
          (*token)[char_position] = (char)(7); // ascii value correspinding to
                                               // \a and similarly for \t \n ...
          break;
        case 'b':
          (*token)[char_position] = (char)(8);
          break;
        case 't':
          (*token)[char_position] = (char)(9);
          break;
        case 'n':
          (*token)[char_position] = (char)(10);
          break;
        case 'v':
          (*token)[char_position] = (char)(11);
          break;
        case 'f':
          (*token)[char_position] = (char)(12);
          break;
        case 'r':
          (*token)[char_position] = (char)(13);
          break;
        case '\\':
          (*token)[char_position] = (char)(92);
          break;
        }
        char_position++;
        cptr++;
      } else {
        cptr--;
        int count = 0;
        int decimal_value = 0;
        // count < 3 is to check the next 3 characters after '\'
        while (count < NUMBER_OF_OCTAL_VALUE_CHARACTERS && *cptr != '\n' &&
               *cptr != ' ') {
          cptr++;
          if (*cptr > '7' || *cptr < '0') // if the character is not an
                                          // octal-digit then the input was not
                                          // an octal value.
          {
            count = 0;
            decimal_value = 0;
            break;
          }
          count++;
          decimal_value +=
              (*cptr - '0') * pow(8, NUMBER_OF_OCTAL_VALUE_CHARACTERS - count);
          // decimal_value is the equivalent decimal value of the octal input.
          // calculated as follows input octal => /abc = 64*a + 8*b + c
        }

        // next if branch taken if all the next three characters are all digits.

        if (count == NUMBER_OF_OCTAL_VALUE_CHARACTERS) {
          (*token)[char_position] =
              (char)decimal_value; // direct conversion decimal to character
          char_position++;
          cptr++;
        }

        else {
          (*token)[char_position] = '\\';
          while (count > 0) {
            cptr--;
            count--;
          }

          char_position++;
        }
      }
    } else {
      (*token)[char_position] = *cptr;
      char_position++;
      cptr++;
    }
  }
  (*token)[char_position] = '\0';
  *bptr = cptr;
}

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

int parseArg(char **arg, char **bptr) {
  // consume starting white spaces
  while (**bptr == ' ')
    (*bptr)++;

  if (**bptr == '\n')
    return SUCCESS;

#if BMRK_C
  // handle string
  if (**bptr == '"') {
    int parse = parseString(arg, bptr);
    if (parse == SUCCESS)
      return PARSED_ARGV;
    else
      return FAIL;
  }

  // handle stdin from a file
  if (**bptr == '<') {
    int parse = parseStdin(arg, bptr);
    if (parse == SUCCESS)
      return PARSED_STDIN;
    else
      return FAIL;
  }
#endif

  // handle others
  copyToken(arg, bptr);

  return PARSED_ARGV;
}

int is_test_chosen(int total_num, int num_tests) {

  int mult = 100000;
  float prob = (float)num_tests / total_num;
  prob *= mult;
  float res = rand() % mult;
  return res < prob;
}

int read_test_cases(struct partecl_input *inputs, int num_test_cases) {
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

  /* TODO:
  // we are using the step to read a distribution of the tests in the file, when we only want a few of them
  int step = 1;
  if (num_test_cases < total_num_tests) {
    step = total_num_tests / num_test_cases;
    if(total_num_tests % num_test_cases != 0) {
      step++;
    }
  }
  */

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
    if(!is_test_chosen(total_num_tests, num_test_cases))
    {
      continue;
    }
    /* TODO
    if(line_index % step != 0) {
      continue;
    }
    */

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

