#include "structs.h"
#include "cl-string.h"
#include "fsm.h"
//#include <stdbool.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

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

  /*printf("No transitions \n");*/
  return -1; // In case of no transition -1 is returned
}

/**
 * Executes the FSM.
 * Returns the final state.
 */

void execute_fsm(struct transition *transitions, int num_transitions,
                 char *input_ptr, int input_length, int output_length) {

  int length = strlen(input_ptr) / input_length * output_length;
  char output[length];
  int extra_length = 0;
  char *output_ptr = output;
  short current_state = transitions[0].current_state;
  /*printf("%d\n", current_state);*/
  while (*input_ptr != '\0') {
    int copy_current_state = current_state; // copy of current_state
    current_state = lookup_symbol(num_transitions, transitions, current_state, input_ptr,
                      input_length, output_ptr);

    if (current_state == -1) {
      extra_length += output_length; // keeping a count of when output is not there.
      current_state = copy_current_state; // resetting the current_state
      input_ptr += input_length; // moving only the input pointer ahead
      continue;
    }
    
    input_ptr += input_length;
    output_ptr += output_length;

    /*printf("%d\n", current_state);*/
  }
  
  for (int i = 0; i < length - extra_length; ++i)
    /*printf("%c", output[i]);*/

  /*printf("\n");*/
}

__kernel void main_kernel(__global struct partecl_input* inputs, __global struct partecl_result* results) {
  int idx = get_global_id(0);
  struct partecl_input input_gen = inputs[idx];
  __global struct partecl_result *result_gen = &results[idx];
  int argc = input_gen.argc;
  result_gen->test_case_num = input_gen.test_case_num;


  if (argc < 3) {
    /*printf("Please provide an input filename and an input sequence.\n");*/
    //return 0;
  }

  // inputs
  char *filename = argv[1];
  char *input_ptr = argv[2];

  // read the fsm
  // transitions
  int num_transitions;
  int input_length;
  int output_length;
  struct transition *transitions =
      read_fsm(filename, &num_transitions, &input_length, &output_length);

  if (transitions != NULL) {
    execute_fsm(transitions, num_transitions, input_ptr, input_length,
                output_length);
  }
  result_gen->final_state = current_state;
  for(int i = 0; i < length; i++)
  {
    result_gen->output[i] = output[i];
  }
  result_gen->length = length;
}
