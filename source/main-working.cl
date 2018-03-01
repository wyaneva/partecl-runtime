#include "structs.h"
#include "cl-stdio.h"
#include "cl-string.h"
#include "fsm.h"
//#include <stdbool.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

/**
 * Read the value of a parameter in the KISS2 file.
 * This refers to the .i, .o, .p and .s parameters
 */

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

  /*printf("\nCouldn't find transition for state %lld, input %s.\n",
         current_state, input);*/
  return -1;
}

/**
 * Executes the FSM.
 * Returns the final state.
 */
__kernel void execute_fsm(__global struct partecl_input *inputs,
                          __global struct partecl_result *results,
                          __global struct transition *transitions,
                          __global int num_transitions,
                          __global int input_length,
                          __global int output_length) {

  int idx = get_global_id(0);
  struct partecl_input input_gen = inputs[idx];
  __global struct partecl_result *result_gen = &results[idx];
  result_gen->test_case_num = input_gen.test_case_num;

  long long int current_state = transitions[0].current_state;
  while (*input_gen.input_ptr != '\0') {
    current_state = lookup_symbol(num_transitions, transitions, current_state,
                                  input_ptr, input_length, output_ptr);

    if (current_state == -1) {
      return -1;
    }

    input_gen.input_ptr += input_length;
    result_gen->output_ptr += output_length;
  }

  //return current_state;
  result_gen->current_state = current_state;
}
