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

#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TRANSITIONS_PER_STATE 256
#define NUMBER_OF_OCTAL_VALUE_CHARACTERS 3

#define FAIL 0
#define SUCCESS 1
#define PARSED_ARGV 2
#define PARSED_STDIN 3

struct transition {

  int start_state;
  int next_state;
  char transition[5];
  char output[2];
};

struct transition_pair {

  int start_state;
  int next_state;
  char transition[9];
  char output[3];
};

struct transition_pair_visit {

  int start_state;
  int next_state;
  int mid_state;
  char transition[9];
  int is_visited;
};

struct aggr {

  int k;
  double M;
  double S;
};

char *read_file(const char *);
char *concatenate_strings(char *, const char*);

int parseArg(char **, char **);
int parseString(char **, char **);
void read_fsm_numbers(const char *, int *, int *, int *, int *);

void copy_word_static(char[], char **);
int calculate_test_length(char *);

void update_aggr(struct aggr *, int);
void finalise_aggr(struct aggr, double *, double *);

int is_test_chosen(int, int);

#endif
