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
short lookup_symbol(transition *transitions, short current_state, char input[],
                    int length, char *output_ptr) {

  int idx = get_index(current_state, input[0]);
  transition trans = transitions[idx];
  strcpy(output_ptr, trans.output);
  return trans.next_state;

  //printf("\nCouldn't find transition for state %d, input %s.\n", current_state,
  //       input);
  //return -1;
}

/**
 * Executes the FSM.
 * Returns the final state.
 */
void execute_fsm(transition *transitions, short starting_state, char *input_ptr,
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

    current_state = lookup_symbol(transitions, current_state, input_ptr,
                                  input_length, output_ptr);

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
  transition *transitions =
      read_fsm(filename, &num_transitions, &starting_state, &input_length, &output_length);

  if (transitions == NULL) {
    /*printf("Reading the FSM failed.");*/
    //return -1;
  }

  execute_fsm(transitions, starting_state, input_ptr, input_length,
              output_length);
  result_gen->final_state = current_state;
  for(int i = 0; i < length; i++)
  {
    result_gen->output[i] = output[i];
  }
  result_gen->length = length;
}
