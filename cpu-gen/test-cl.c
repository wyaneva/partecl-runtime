#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../source/constants.h"
#include "../kernel-gen/fsm.h"
#include "../kernel-gen/structs.h"

/**
 * Looksup an FSM input symbol, given the symbol and the current state.
 * Returns the next state or -1 if transition isn't found.
 */
short lookup_symbol(transition *transitions, short current_state, char input[],
                    int length, char *output_ptr) {

  int idx = get_index(current_state, input[0]);
  transition trans = transitions[idx];

  if (trans.next_state == -1) {
    printf("\nCouldn't find transition for state %d, input %s.\n",
           current_state, input);
  }

  strcpy(output_ptr, trans.output);
  return trans.next_state;
}

/**
 * Executes the FSM.
 * Returns the final state.
 */

#if FSM_INPUTS_WITH_OFFSETS
void run_main(char *input, char *result, transition *transitions,
              short starting_state, int input_length, int output_length) {
#else
void run_main(struct partecl_input input, struct partecl_result *result,
              transition *transitions, short starting_state, int input_length,
              int output_length) {
#endif

#if FSM_INPUTS_WITH_OFFSETS
  char *input_ptr = &input[0];
  char *output_ptr = &result[0];
#else
  char *input_ptr = input.input_ptr;
  char *output_ptr = result->output;
#endif

  // output
  short current_state = starting_state; // transitions[0].current_state;
  while (*input_ptr != '\0') {

    if (current_state == -1) {
      return;
    }

    current_state = lookup_symbol(transitions, current_state, input_ptr,
                                  input_length, output_ptr);

    input_ptr += input_length;
    output_ptr += output_length;
  }
}
