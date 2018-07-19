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
                    char input, int length, TEST_OUTPUTS_ATTR char *output_ptr) {

  int index = get_index(current_state, input);
  transition trans = transitions[index];

  if (trans.next_state == -1) {
    printf("\nCouldn't find transition for state %d, input %c.\n",
           current_state, input);
    return current_state;
  }

  *output_ptr = *(trans.output);
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
                        FSM_ATTR_KNL transition *transitions_knl,
                        int starting_state, int input_length, int output_length,
                        int num_test_cases) {
#else
#if FSM_INPUTS_COAL_CHAR || FSM_INPUTS_COAL4
kernel void execute_fsm(global TEST_INPUTS_TYPE *inputs,
                        global TEST_INPUTS_TYPE *results,
                        FSM_ATTR_KNL transition *transitions_knl,
                        int starting_state, int input_length, int output_length,
                        int num_test_cases) {
#else
kernel void execute_fsm(global TEST_INPUTS_TYPE *inputs,
                        global TEST_INPUTS_TYPE *results,
                        FSM_ATTR_KNL transition *transitions_knl,
                        int starting_state, int input_length, int output_length,
                        int num_test_cases, int padded_input_size) {
#endif
#endif

  int idx = get_global_id(0);
  int num_threads = get_global_size(0);

  // FSM
#if FSM_LOCAL_MEMORY
  // copy FSM into local memory
  int idx_local = get_local_id(0);
  size_t local_size = get_local_size(0);
  local transition transitions_local[NUM_TRANSITIONS_KERNEL];
  for (int i = idx_local; i < NUM_TRANSITIONS_KERNEL; i += local_size) {
    transitions_local[i] = transitions_knl[i];
  }
  FSM_ATTR transition *transitions = transitions_local;
#else
  FSM_ATTR transition *transitions = transitions_knl;
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
  int coal_idx = idx*padded_input_size;
  global TEST_INPUTS_TYPE *input_ptr = &inputs[coal_idx];
  global TEST_INPUTS_TYPE *output_ptr = &results[coal_idx];
#endif
#endif

  // execute
  short current_state = starting_state;
#if FSM_INPUTS_COAL_CHAR4
  while (true) {
#else
  while (*input_ptr != '\0') {
#endif

#if FSM_INPUTS_COAL_CHAR4
    char output_temp;
    if ((*input_ptr).x == '\0') {
      (*output_ptr).x = '\0';
      break;
    }
    current_state = lookup_symbol(transitions, current_state, (*input_ptr).x,
                                  input_length, &output_temp);
    (*output_ptr).x = output_temp;

    if ((*input_ptr).y == '\0') {
      (*output_ptr).y = '\0';
      break;
    }
    current_state = lookup_symbol(transitions, current_state, (*input_ptr).y,
                                  input_length, &output_temp);
    (*output_ptr).y = output_temp;

    if ((*input_ptr).z == '\0') {
      (*output_ptr).z = '\0';
      break;
    }
    current_state = lookup_symbol(transitions, current_state, (*input_ptr).z,
                                  input_length, &output_temp);
    (*output_ptr).z = output_temp;

    if ((*input_ptr).w == '\0') {
      (*output_ptr).w = '\0';
      break;
    }
    current_state = lookup_symbol(transitions, current_state, (*input_ptr).w,
                                  input_length, &output_temp);
    (*output_ptr).w = output_temp;
#else
    current_state = lookup_symbol(transitions, current_state, (*input_ptr),
                                  input_length, output_ptr);
#endif

#if FSM_INPUTS_COAL_CHAR || FSM_INPUTS_COAL_CHAR4
    input_ptr += input_length * num_threads;
    output_ptr += output_length * num_threads;
#else
    input_ptr += input_length;
    output_ptr += output_length;
#endif
  }

#if !FSM_INPUTS_COAL_CHAR4
  *output_ptr ='\0';
#endif
}
