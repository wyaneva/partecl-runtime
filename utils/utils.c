/*
 * Copyright 2019 Vanya Yaneva, The University of Edinburgh
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

#include "utils.h"
#include <assert.h>
#include <math.h>
#include <regex.h>

char *read_file(const char *filename) {
  FILE *f = fopen(filename, "r");
  if (!f) {
    printf("Reading file %s \n", filename);
    perror(" ERROR");
    return NULL;
  }

  // figure out file size
  fseek(f, 0, SEEK_END);
  size_t size = ftell(f);

  fseek(f, 0, SEEK_SET);

  // allocate memory, slurp in entire file
  char *result = (char *)malloc(size + 1);
  fread(result, 1, size, f);

  // close, return
  fclose(f);
  result[size] = '\0';

  return result;
}

char *concatenate_strings(char *str1, const char* str2) {

  int length = strlen(str2);
  sprintf(str1, "%s", str2);

  return str1+length;
}

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
#endif

  // handle others
  copyToken(arg, bptr);

  return PARSED_ARGV;
}


void copy_word_static(char token[], char **bptr) {
  // create current ptr
  char *cptr = *bptr;
  int char_position = 0;

  while (*cptr != '\n' && *cptr != ' ') {

    token[char_position] = *cptr;
    char_position++;
    cptr++;
  }

  token[char_position] = '\0';
  *bptr = cptr + 1;
}

void read_fsm_numbers(const char *filename, int *num_trans_pairs,
                      int *num_states, int *num_trans, int *start_state) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    printf("Could not find tile with transition_pairs %s.\n", filename);
    return;
  }

  char line[100];

  bool found_trans_pairs = num_trans_pairs == NULL;
  bool found_states = num_states == NULL;
  bool found_trans = num_trans == NULL;
  bool found_start_state = start_state == NULL;

  while (fgets(line, sizeof(line), file) != NULL) {

    if (found_trans_pairs && found_states && found_trans && found_start_state)
      break;

    // get the first token
    char *token = strtok(line, " ");
    if (!found_trans_pairs && token[0] == '.' && token[1] == 't') {

      token = strtok(NULL, " ");
      *num_trans_pairs = strtol(token, NULL, 10);
      found_trans_pairs = 1;

    } else if (!found_states && token[0] == '.' && token[1] == 's') {

      token = strtok(NULL, " ");
      *num_states = strtol(token, NULL, 10);
      (*num_states)++; // adding one as sometimes the index of the last state is
                       // the same as the number
      found_states = 1;

    } else if (!found_trans && token[0] == '.' && token[1] == 'p') {

      token = strtok(NULL, " ");
      *num_trans = strtol(token, NULL, 10);
      found_trans = 1;

    } else if (!found_start_state) {

      token = strtok(NULL, " ");
      *start_state = strtol(token, NULL, 10);
      token = strtok(NULL, " ");
      if (token != NULL)
        found_start_state = 1;
    }
  }

  fclose(file);
}

int find_num_matches(regex_t *pexp, char *sz) {
  // we just need the whole string match in this example
  regmatch_t whole_match;
  int num_matches = 0;

  // we store the eflags in a variable, so that we can make
  // ^ match the first time, but not for subsequent regexecs
  int eflags = 0;
  int match = 0;
  size_t offset = 0;
  size_t length = strlen(sz);

  while (regexec(pexp, sz + offset, 1, &whole_match, eflags) == 0) {
    // do not let ^ match again.
    eflags = REG_NOTBOL;
    match = 1;
    // printf("range %zd - %zd matches\n", offset + whole_match.rm_so,
    //       offset + whole_match.rm_eo);

    num_matches++;

    // increase the starting offset
    offset += whole_match.rm_eo;

    // a match can be a zero-length match, we must not fail
    // to advance the pointer, or we'd have an infinite loop!
    if (whole_match.rm_so == whole_match.rm_eo) {
      offset += 1;
    }

    // break the loop if we've consumed all characters. Note
    // that we run once for terminating null, to let
    // a zero-length match occur at the end of the string.
    if (offset > length) {
      break;
    }
  }
  if (!match) {
    // printf("\"%s\" does not contain a match\n", sz);
  }

  return num_matches;
}

int calculate_test_length(char *testinput) {

  int length = strlen(testinput);

  regex_t octal_regex;
  regex_t escaped_regex;
  int reti;

  // find characters of type \012 - they are counted as one character
  reti = regcomp(&octal_regex, "\\\\[0-7][0-7][0-7]", 0);
  if (reti) {
    printf("Could not compile octal regular expression!\n");
    return -1;
  }
  int num_octals = find_num_matches(&octal_regex, testinput);
  regfree(&octal_regex);

  // find escaped characters
  reti = regcomp(&escaped_regex, "\\\\[abtnvfr\\\\]", 0);
  if (reti) {
    printf("Could not compile octal regular expression!\n");
    return -1;
  }
  int num_escaped = find_num_matches(&escaped_regex, testinput);
  regfree(&escaped_regex);

  length -= num_octals * 3 + num_escaped;
  return length;
}

/*
 Welford's Algorithm to compute the running mean (average) and variance
 from: github.com/alexalemi/welford.py
*/
void update_aggr(struct aggr *existing_aggr, int new_value) {

  if (existing_aggr == NULL) {
    return;
  }

  int k = existing_aggr->k;
  double M = existing_aggr->M;
  double S = existing_aggr->S;

  k++;
  double new_M = M + (new_value - M) / k;
  double new_S = S + (new_value - M) * (new_value - new_M);

  existing_aggr->k = k;
  existing_aggr->M = new_M;
  existing_aggr->S = new_S;
}

// retrieve the mean and sd from the existing aggregate
void finalise_aggr(struct aggr existing_aggr, double *mean, double *sd) {

  int k = existing_aggr.k;
  double M = existing_aggr.M;
  double S = existing_aggr.S;

  if (k < 2)
    return;

  *mean = M;
  *sd = sqrt(S / (k - 1));
}

/*
 END of Welford's Algorithm
*/
