#include "cl-stdio.h"
#include "cl-string.h"
#include "fsm.h"
#include "structs.h"
//#include <stdbool.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#define NUM_TRANSITIONS 1096
#define OUTPUT_LENGTH 300
#define INPUT_LENGTH 300

/**
 * Read the value of a parameter in the KISS2 file.
 * This refers to the .i, .o, .p and .s parameters
 */

bool comparebinary(local char binary1[], char binary2[], int length) {

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
short lookup_symbol(int num_transitions, local struct transition transitions[],
                    short current_state, local char input[], int length,
                    private char *output_ptr) {

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
kernel void execute_fsm(global struct partecl_input *inputs,
                        global struct partecl_result *results,
                        global struct transition *transitions,
                        int num_transitions, int input_length,
                        int output_length) {

  int idx = get_global_id(0);
  struct partecl_input input_gen = inputs[idx];
  global struct partecl_result *result_gen = &results[idx];
  result_gen->test_case_num = input_gen.test_case_num;

  //copy inputs into local memory
  local char input_local[INPUT_LENGTH];
  local char *input_ptr = &input_local[0];
  char *input_global_ptr = &input_gen.input_ptr[0];
  while (*input_global_ptr != '\0') {
    *input_ptr = *input_global_ptr;
    input_ptr++;
    input_global_ptr++;
  }
  *input_ptr = '\0';
  input_ptr = &input_local[0];

  //copy FSM into local memory
  local struct transition transitions_local[NUM_TRANSITIONS];
  for (int i = 0; i < num_transitions; i++) {
    transitions_local[i] = transitions[i];
  }

  // output
  int length = (strlen(input_ptr) / input_length) * output_length;
private
  char output[OUTPUT_LENGTH];
private
  char *output_ptr = output;

  printf("%d %d %d\n", input_gen.test_case_num, length, strlen(input_ptr));

  short current_state = transitions[0].current_state;
  while (*input_ptr != '\0') {
    current_state =
        lookup_symbol(num_transitions, transitions_local, current_state,
                      input_ptr, input_length, output_ptr);

    if (current_state == -1) {
      // return;
    }

    input_ptr += input_length;
    output_ptr += output_length;
  }

  // print the output
  //for (int i = 0; i < length; i++) {
    // printf("%c", output[i]);
  //}
  // printf("\n");
  // printf("Final state: %ld\n", current_state);

  for (int i = 0; i < length; i++) {
    result_gen->output[i] = output[i];
  }
  result_gen->length = length;
  result_gen->final_state = current_state;
}
