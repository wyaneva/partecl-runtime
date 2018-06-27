#include "cl-stdio.h"
#include "cl-string.h"
#include "fsm.h"
#include "structs.h"
#include "constants.h"
//#include <stdbool.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

/**
 * Read the value of a parameter in the KISS2 file.
 * This refers to the .i, .o, .p and .s parameters
 */
bool compare_inputs(TEST_INPUTS_ATTR char test_input[], char transition_input[],
                    int length) {

#if BMRK_OS
  char anychar = '-'; // '-' denotes ANY bit in the KISS2 format
#endif

  for (int i = 0; i < length; i++) {
    if (test_input[i] == '\0' || transition_input[i] == '\0') {
      return false;
    }

#if BMRK_OS
    if (test_input[i] == anychar || transition_input[i] == anychar) {
      continue;
    }
#endif

#if BMRK_NETWORK
    if (test_input[i] == '\n') {
      continue;
    }
#endif

    if (test_input[i] != transition_input[i])
      return false;
  }
  return true;
}

/**
 * Looksup an FSM input symbol, given the symbol and the current state.
 * Returns the next state or -1 if transition isn't found.
 */
short lookup_symbol(FSM_ATTR transition transitions[], int start, int end,
                    short current_state, TEST_INPUTS_ATTR char input[],
                    int length, private char *output_ptr) {

  for (int i = start; i < end; i++) {
    transition trans = transitions[i];
    if (compare_inputs(input, trans.input, length)) {
      strcpy(output_ptr, trans.output);
      return trans.next_state;
    }
  }

  printf("\nCouldn't find transition for state %d, input %s.\n",
         current_state, input);
  return -1;
}

/**
 * Executes the FSM.
 * Returns the final state.
 */

#if FSM_OPTIMISE_COAL
kernel void
execute_fsm(global char *inputs, global struct partecl_result *results,
            FSM_ATTR_KNL transition *transitions, FSM_ATTR_KNL int *offsets,
            FSM_ATTR_KNL int *num_transitions_per_state, int starting_state,
            int input_length, int output_length, int num_test_cases) {
#else
kernel void execute_fsm(global struct partecl_input *inputs,
                        global struct partecl_result *results,
                        FSM_ATTR_KNL transition *transitions,
                        FSM_ATTR_KNL int *offsets,
                        FSM_ATTR_KNL int *num_transitions_per_state,
                        int starting_state, int input_length, int output_length,
                        int num_test_cases) {
#endif

  int idx = get_global_id(0);

#if FSM_OPTIMISE_COAL
#else
  struct partecl_input input_gen = inputs[idx];
#endif

  global struct partecl_result *result_gen = &results[idx];
  result_gen->test_case_num = idx + 1;

#if FSM_LOCAL_MEMORY
  // copy FSM into local memory
  local transition transitions_local[NUM_TRANSITIONS_KERNEL];
  for (int i = 0; i < NUM_TRANSITIONS_KERNEL; i++) {
    transitions_local[i] = transitions[i];
  }
#endif

  // input
#if FSM_OPTIMISE_COAL
  global char *input_ptr = &inputs[idx * input_length];
#else
  char *input_ptr = input_gen.input_ptr;
#endif
  //keep this comment

  // output
  // int length = (strlen(input_ptr) / input_length) * output_length;
private
  char output[OUTPUT_LENGTH_KERNEL];
private
  char *output_ptr = output;

  short current_state = starting_state; // transitions[0].current_state;
  while (*input_ptr != '\0') {

    if (current_state == -1) {
      //return;
    }

    int start = offsets[current_state];
    int end = start + num_transitions_per_state[current_state];
#if FSM_LOCAL_MEMORY
    current_state = lookup_symbol(transitions_local, start, end, current_state,
                                  input_ptr, input_length, output_ptr);
#else
    current_state = lookup_symbol(transitions, start, end, current_state,
                                  input_ptr, input_length, output_ptr);
#endif

#if FSM_OPTIMISE_COAL
    input_ptr += input_length * num_test_cases;
#else
    input_ptr += input_length;
#endif
    output_ptr += output_length;
  }

  int length = strlen(output);

  for (int i = 0; i < length; i++) {
    result_gen->output[i] = output[i];
  }
  result_gen->length = length;
  result_gen->final_state = current_state;
}
