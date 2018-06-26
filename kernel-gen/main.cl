#include "structs.h"
#include "cl-string.h"
#include "fsm.h"
//#include <stdbool.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

bool compare_inputs(char binary1[], char binary2[], int length) {

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
short lookup_symbol(transition *transitions, int start, int end, short current_state, char input[],
                    int length, char *output_ptr) {

  for (int i = start; i < end; i++) {
    transition trans = transitions[i];
    if (compare_inputs(input, trans.input, length)) {
      strcpy(output_ptr, trans.output);
      return trans.next_state;
    }
  }

  /*printf("\nCouldn't find transition for state %d, input %s.\n", current_state,
         input);*/
  return -1;
}

/**
 * Executes the FSM.
 * Returns the final state.
 */
void execute_fsm(transition *transitions, int offsets[],
                 int num_transitions_per_state[], short starting_state, char *input_ptr,
                 int input_length, int output_length) {

  // output
  int length = strlen(input_ptr) / input_length * output_length;
  char output[length];
  char *output_ptr = output;

  short current_state = starting_state; // transitions[0].current_state;
  while (*input_ptr != '\0') {

    if (current_state == -1) {
      return;
    }

    int start = offsets[current_state];
    int end = start + num_transitions_per_state[current_state];
    current_state = lookup_symbol(transitions, start, end, current_state,
                                  input_ptr, input_length, output_ptr);

    input_ptr += input_length;
    output_ptr += output_length;
  }

  // print the output
  for (int i = 0; i < length; i++) {
    /*printf("%c", output[i]);*/
  }
  /*printf("\n");*/
  /*printf("Final state: %d\n", current_state);*/
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
  int starting_state;
  int input_length;
  int output_length;
  int *num_transitions_per_state = (int *)malloc(sizeof(int) * NUM_STATES);
  transition *transitions_unopt =
      read_fsm(filename, num_transitions_per_state, &num_transitions, &starting_state,
               &input_length, &output_length);

  if (transitions_unopt == NULL) {
    /*printf("Reading the FSM failed.");*/
    //return -1;
  }

  transition *transitions =
      (transition *)malloc(sizeof(transition) * num_transitions);
  int offsets[NUM_STATES];

  int idx = 0;
  int current_offset = 0;
  for (int i = 0; i < NUM_STATES; i++) {
    for (int j = 0; j < MAX_NUM_TRANSITIONS_PER_STATE; j++) {
      if (j >= num_transitions_per_state[i])
        break;

      transition current_trans = transitions_unopt[i * MAX_NUM_TRANSITIONS_PER_STATE + j];

      transitions[idx] = current_trans;
      idx++;
    }

    int prev = i > 0 ? i - 1 : 0;
    offsets[i] = current_offset + num_transitions_per_state[prev];
    current_offset = offsets[i];
  }

  execute_fsm(transitions, offsets, num_transitions_per_state, starting_state,
              input_ptr, input_length, output_length);
  result_gen->final_state = current_state;
  for(int i = 0; i < length; i++)
  {
    result_gen->output[i] = output[i];
  }
  result_gen->length = length;
}
