#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../kernel-gen/fsm.h"
#include "../kernel-gen/structs.h"

bool compare_inputs(char test_input[], char transition_input[], int length) {

  char anychar = '-'; // '-' denotes ANY bit in the KISS2 format
  for (int i = 0; i < length; i++) {
    if (test_input[i] == '\0' || transition_input[i] == '\0')
      return false;
    if (test_input[i] == anychar || transition_input[i] == anychar)
      continue;

    if (test_input[i] != transition_input[i])
      return false;
  }
  return true;
}

/**
 * Looksup an FSM input symbol, given the symbol and the current state.
 * Returns the next state or -1 if transition isn't found.
 */
short lookup_symbol(int num_transitions, struct transition transitions[],
                    short current_state, char input[], int input_length,
                    char *output_ptr) {
  for (int i = 0; i < num_transitions; i++) {
    struct transition trans = transitions[i];
    if (trans.current_state == current_state &&
        compare_inputs(input, trans.input, input_length)) {
      strcpy(output_ptr, trans.output);
      return trans.next_state;
    }
  }

  printf("No transitions \n");
  return -1; // In case of no transition -1 is returned
}

/**
 * Executes the FSM.
 * Returns the final state.
 */
void run_main(struct partecl_input input, struct partecl_result *result,
                 struct transition *transitions, int num_transitions,
                 int input_length, int output_length) {

  char* input_ptr = input.input_ptr;
  char* output_ptr = result->output;

  // output
  int length = strlen(input_ptr) / input_length * output_length;
  //char output[length];
  //char *output_ptr = output;

  short current_state = transitions[0].current_state;
  while (*input_ptr != '\0') {
    current_state = lookup_symbol(num_transitions, transitions, current_state,
                                  input_ptr, input_length, output_ptr);

    if (current_state == -1) {
      return;
    }

    input_ptr += input_length;
    output_ptr += output_length;
  }

  // print the output
  /*
  for (int i = 0; i < length; i++) {
    printf("%c", output[i]);
  }
  printf("\n");
  printf("Final state: %ld\n", current_state);
  */

  result->length=length;
  result->final_state = current_state;
}
