#include "fsm.h"
//#include <math.h>
//#include <stdbool.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#define NUMBER_OF_OCTAL_VALUE_CHARACTERS 3

void copy_word(char **token, char **bptr) {
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

void copy_word_static(char token[], char **bptr) {
  // create current ptr
  char *cptr = *bptr;
  int char_position = 0;

  while (*cptr != '\n' && *cptr != ' ') {
    // non-printable characters are encoded as \[0-7][0-7][0-7] backslash and 3
    // octal digits.
    if (*cptr == '\\') {
      cptr++;
      if (*cptr == 'a' || *cptr == 'b' || *cptr == 't' || *cptr == 'n' ||
          *cptr == 'v' || *cptr == 'f' || *cptr == 'r') {
        switch (*cptr) {
        case 'a':
          token[char_position] = (char)(7); // ascii value correspinding to \a
                                           // and similarly for \t \n ...
          break;
        case 'b':
          token[char_position] = (char)(8);
          break;
        case 't':
          token[char_position] = (char)(9);
          break;
        case 'n':
          token[char_position] = (char)(10);
          break;
        case 'v':
          token[char_position] = (char)(11);
          break;
        case 'f':
          token[char_position] = (char)(12);
          break;
        case 'r':
          token[char_position] = (char)(13);
          break;
        }
        char_position++;
        cptr++;
      } else {
        cptr--;
        int count = 0;
        int decimal_value = 0;
        // count < 3 is to check the next 3 characters after '\'
        while (count < NUMBER_OF_OCTAL_VALUE_CHARACTERS && *cptr != '\n' && *cptr != ' ')
        {
          cptr++;
          if (*cptr > '7' || *cptr < '0') // if the character is not an octal-digit
                                          // then the input was not an octal
                                          // value.
          {
            count = 0;
            decimal_value = 0;
            break;
          }
          count++;
          decimal_value += (*cptr - '0') * pow(8, NUMBER_OF_OCTAL_VALUE_CHARACTERS - count); 
          // decimal_value is the equivalent decimal value of the octal input. calculated as follows
          // input octal => /abc = 64*a + 8*b + c                                                                                                      
        }                             
        
        // next if branch taken if all the next three characters are all digits.

        if (count == NUMBER_OF_OCTAL_VALUE_CHARACTERS ) {
          token[char_position] = (char)decimal_value; // direct conversion decimal to character
          char_position++;
          cptr++;
        }

        else {
          token[char_position] = '\\';
          while (count > 0) {
            cptr--;
            count--;
          }

          char_position++;
        }
      }
    } else {
      token[char_position] = *cptr;
      char_position++;
      cptr++;
    }
  }
  token[char_position] = '\0';
  *bptr = cptr + 1;
}

short state_to_decimal(char *state, int state_base) {

  char *stateptr = state;
  short decimal = 0;
  while (*stateptr != '\0') {
    decimal = decimal * state_base + *stateptr - '0';
    stateptr++;
  }

  return decimal;
}

/**
 * Read the value of a parameter in the KISS2 file.
 * This refers to the .i, .o, .p and .s parameters.
 * We have also added .b parameter, which tells us in what numerical base state
 * numbers are encoded.
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
  case STATE_BASE:
    param_char = 'b';
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
      copy_word(&valueptr, &lineptr);

      param_value = strtol(valueptr, NULL, 10);
      break;
    }
  }

  return param_value;
}

/**
 * Read an FSM from a KISS2 file
 */
struct transition *read_fsm(const char *filename, int *num_transitions,
                            int *input_length, int *output_length) {

  // read the parameters
  // number of states
  *num_transitions = read_parameter(filename, NUM_TRANSITIONS);
  if (*num_transitions == -1) {
    /*printf("File %s does not specify a number of transitions. Exiting. \n",
           filename);*/
    return NULL;
  }

  // input length
  *input_length = read_parameter(filename, INPUT_LENGTH);
  if (*input_length == -1) {
    /*printf("File %s does not specify input length. Exiting. \n", filename);*/
    return NULL;
  }

  // output length
  *output_length = read_parameter(filename, OUTPUT_LENGTH);
  if (*output_length == -1) {
    /*printf("File %s does not specify output length. Exiting. \n", filename);*/
    return NULL;
  }

  // state base
  int state_base = read_parameter(filename, STATE_BASE);
  if (state_base == -1) {
    /*printf("File %s does not specify the state base. Exiting. \n", filename);*/
    return NULL;
  }

  // open the file
  char line[1000];
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    /*printf("Could not find FSM file %s.\n", filename);*/
    return NULL;
  }

  // read transitions line for line
  struct transition *transitions = (struct transition *)malloc(
      sizeof(struct transition) * (*num_transitions));
  struct transition *transptr = &transitions[0];
  while (fgets(line, sizeof(line), file) != NULL && transptr != NULL) {

    char *lineptr = &line[0];
    if (*lineptr == '\n' || *lineptr == ' ') {
      // skip empty lines 
      continue;
    }

    if(*lineptr == '.' && (*(lineptr+1) != ' ')) {
      // skip formatting lines
      continue;
    }

    // read transition line token by token
    char *curtoken;

    // input
    copy_word_static(transptr->input, &lineptr);

    // current state
    copy_word(&curtoken, &lineptr);
    transptr->current_state = state_to_decimal(curtoken, state_base);

    // next state
    copy_word(&curtoken, &lineptr);
    transptr->next_state = state_to_decimal(curtoken, state_base);

    // output
    copy_word_static(transptr->output, &lineptr);

    transptr++;
  }

  return transitions;
}
