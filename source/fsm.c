#include <stdio.h>
#include <stdlib.h>
#include "../kernel-gen/fsm.h"

void copyWord(char **token, char **bptr) {
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
  *bptr = cptr + 1;
}

void copyWordStatic(char token[], char **bptr) {
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
  for (int i = 0; i < num_chars; i++) {
    token[i] = *cptr;
    cptr++;
  }
  token[num_chars] = '\0';
  *bptr = cptr + 1;
}

int read_parameter(const char *filename, enum fsm_parameter param_type) {

  char param_char;
  switch (param_type) {
  case INPUT_LENGTH:
    param_char = 'i';
    break;
  case OUTPUT_LENGTH:
    param_char = 'o';
    break;
  case NUM_TRANSITIONS:
    param_char = 'p';
    break;
  }

  // open the file
  char line[1000];
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    /*printf("Could not find FSM file %s.\n", filename);*/
    return -1;
  }

  int param_value = -1;
  while (fgets(line, sizeof(line), file) != NULL) {
    if (line[0] == '.' && line[1] == param_char) {
      char *valueptr;
      char *lineptr = &line[3];
      copyWord(&valueptr, &lineptr);

      param_value = strtol(valueptr, NULL, 10);
      break;
    }
  }

  return param_value;
}

long long int statetodecimal(char *state) {

  char *stateptr = state;
  long long int decimal = 0;
  while (*stateptr != '\0') {
    decimal = decimal * 2 + *stateptr - '0';
    stateptr++;
  }

  return decimal;
}

/**
 * Read an FSM from a KISS2 file
 */
bool read_fsm(const char *filename, struct transition *transitions) {

  // open the file
  char line[1000];
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    /*printf("Could not find FSM file %s.\n", filename);*/
    return false;
  }

  // read transitions line for line
  struct transition *transptr = &transitions[0];
  while (fgets(line, sizeof(line), file) != NULL && transptr != NULL) {

    char *lineptr = &line[0];
    if (*lineptr == '\n' || *lineptr == ' ' || *lineptr == '.') {
      // skip empty lines and formatting lines
      continue;
    }

    // read transition line token by token
    char *curtoken;

    // input
    copyWordStatic(transptr->input, &lineptr);

    // current state
    copyWord(&curtoken, &lineptr);
    transptr->current_state = statetodecimal(curtoken);

    // next state
    copyWord(&curtoken, &lineptr);
    transptr->next_state = statetodecimal(curtoken);

    // output
    copyWordStatic(transptr->output, &lineptr);

    transptr++;
  }

  return true;
}

