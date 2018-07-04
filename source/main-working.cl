#include "cl-stdio.h"
#include "cl-string.h"
#include "fsm.h"
#include "structs.h"
#include "constants.h"
//#include <stdbool.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

int char_to_int(const char c) { return (unsigned char)c; }

int get_index(short current_state, char input) {
  int idx = char_to_int(input);
  return current_state * MAX_NUM_TRANSITIONS_PER_STATE + idx;
}

/**
 * Looksup an FSM input symbol, given the symbol and the current state.
 * Returns the next state or -1 if transition isn't found.
 */
short lookup_symbol(FSM_ATTR transition transitions[], short current_state,
                    char input, int length, global char *output_ptr) {

  int index = get_index(current_state, input);
  transition trans = transitions[index];

  if (trans.next_state == -1) {
    printf("\nCouldn't find transition for state %d, input %s.\n",
           current_state, input);
  }

  *output_ptr = *(trans.output);
  //strcpy_global(output_ptr, trans.output);
  return trans.next_state;
}

/**
 * Executes the FSM.
 * Returns the final state.
 */

#if FSM_INPUTS_WITH_OFFSETS
kernel void execute_fsm(global char *inputs,
                        global char *results,
                        global int *offsets,
                        FSM_ATTR_KNL transition *transitions,
                        int starting_state, int input_length, int output_length,
                        int num_test_cases) {
#else
#if FSM_INPUTS_COAL_CHAR || FSM_INPUTS_COAL_CHAR4
kernel void execute_fsm(global TEST_INPUTS_TYPE *inputs,
                        global TEST_INPUTS_TYPE *results,
                        FSM_ATTR_KNL transition *transitions,
                        int starting_state, int input_length, int output_length,
                        int num_test_cases) {
#else
kernel void execute_fsm(global struct partecl_input *inputs,
                        global struct partecl_result *results,
                        FSM_ATTR_KNL transition *transitions,
                        int starting_state, int input_length, int output_length,
                        int num_test_cases) {
#endif
#endif

  int idx = get_global_id(0);

  // FSM
#if FSM_LOCAL_MEMORY
  // copy FSM into local memory
  local transition transitions_local[NUM_TRANSITIONS_KERNEL];
  for (int i = 0; i < NUM_TRANSITIONS_KERNEL; i++) {
    transitions_local[i] = transitions[i];
  }
#endif

  // input
#if FSM_INPUTS_WITH_OFFSETS
  int offset = offsets[idx];
  global char *input_ptr = &inputs[offset];
  global char *output_ptr = &results[offset];
#else

#if FSM_INPUTS_COAL_CHAR || FSM_INPUTS_COAL_CHAR4
  int coal_idx = idx;
  global TEST_INPUTS_TYPE *input_ptr = &inputs[coal_idx];
  global TEST_INPUTS_TYPE *output_ptr = &results[coal_idx];
#else
  struct partecl_input input_gen = inputs[idx];
  char *input_ptr = input_gen.input_ptr;

  global struct partecl_result *result_gen = &results[idx];
  global char *output_ptr = result_gen->output;
#endif

#endif
  //keep this comment

  // execute
  short current_state = starting_state;
#if FSM_INPUTS_COAL_CHAR4
  while ((*input_ptr).x != '\0') {
#else
  while (*input_ptr != '\0') {
#endif

#if FSM_INPUTS_COAL_CHAR4
    current_state = lookup_symbol(transitions, current_state, (*input_ptr).x,
                                  input_length, output_ptr);
    output_ptr += output_length;

    if (input_ptr[0].y == '\0') break;
    current_state = lookup_symbol(transitions, current_state, (*input_ptr).y,
                                  input_length, output_ptr);
    output_ptr += output_length;

    if (input_ptr[0].z == '\0') break;
    current_state = lookup_symbol(transitions, current_state, (*input_ptr).z,
                                  input_length, output_ptr);
    output_ptr += output_length;

    if (input_ptr[0].w == '\0') break;
    current_state = lookup_symbol(transitions, current_state, (*input_ptr).w,
                                  input_length, output_ptr);
    output_ptr += output_length;
#else
    current_state = lookup_symbol(transitions, current_state, (*input_ptr),
                                  input_length, output_ptr);
#if FSM_INPUTS_COAL_CHAR
    output_ptr += output_length * num_test_cases;
#else
    output_ptr += output_length;
#endif

#endif

#if FSM_INPUTS_COAL_CHAR || FSM_INPUTS_COAL_CHAR4
    input_ptr += input_length * num_test_cases;
#else
    input_ptr += input_length;
#endif
  }

#if !FSM_INPUTS_WITH_OFFSETS && !FSM_INPUTS_COAL_CHAR && !FSM_INPUTS_COAL_CHAR4
  // results
  result_gen->length = strlen_global(result_gen->output);
#endif
}
