#include "fsm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 * Read the value of a parameter in the KISS2 file.
 * This refers to the .i, .o, .p and .s parameters
 */
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
    printf("Could not find FSM file %s.\n", filename);
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

/**
 * Read an FSM from a KISS2 file
 */
bool read_fsm(const char *filename, struct transition *transitions) {

  // open the file
  char line[1000];
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    printf("Could not find FSM file %s.\n", filename);
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
    copyWord(&transptr->input, &lineptr);

    // current state
    copyWord(&curtoken, &lineptr);
    transptr->current_state = statetodecimal(curtoken);

    // next state
    copyWord(&curtoken, &lineptr);
    transptr->next_state = statetodecimal(curtoken);

    // output
    copyWord(&transptr->output, &lineptr);

    transptr++;
  }

  return true;
}

bool comparebinary(char binary1[], char binary2[], int length) {

  char anychar = '-'; // '-' denotes ANY bit in the KISS2 format

  for (int i = 0; i < length; i++) {

    if (binary1[i] == '\0' || binary2[i] == '\0')
      return false;

    if (binary1[i] == anychar || binary2[i] == anychar)
      continue;

    if (binary1[i] != binary2[i])
      return false;
  }

  return true;
}

/**
 * Looksup an FSM input symbol, given the symbol and the current state.
 * Returns the next state or -1 if transition isn't found.
 */
long long int lookup_symbol(int num_transitions,
                            struct transition transitions[],
                            long long int current_state, char input[],
                            int length, char *output_ptr) {

  for (int i = 0; i < num_transitions; i++) {

    struct transition trans = transitions[i];
    if (trans.current_state == current_state &&
        comparebinary(input, trans.input, length)) {
      strcpy(output_ptr, trans.output);
      return trans.next_state;
    }
  }

  printf("\nCouldn't find transition for state %lld, input %s.\n",
         current_state, input);
  return -1;
}

/**
 * Executes the FSM.
 * Returns the final state.
 */
long long int execute_fsm(struct transition transitions[], int num_transitions,
                          char *input_ptr, int input_length, char *output_ptr,
                          int output_length) {


  long long int current_state = transitions[0].current_state;
  while (*input_ptr != '\0') {
    current_state = lookup_symbol(num_transitions, transitions, current_state,
                                  input_ptr, input_length, output_ptr);

    if (current_state == -1) {
      return -1;
    }

    input_ptr += input_length;
    output_ptr += output_length;
  }

  return current_state;
}

int run_main(struct partecl_input input, struct partecl_result *result,
             struct transition *transitions, int num_transitions,
             int input_length, int output_length) {

  char* input_ptr = input.input_ptr;

  // output
  int length = strlen(input_ptr) / input_length * output_length;
  char output[length];
  char *output_ptr = result->output;

  long int final_state =
      execute_fsm(transitions, num_transitions, input_ptr, input_length,
                  output_ptr, output_length);

  result->length = length;
  result->final_state = final_state;
}
