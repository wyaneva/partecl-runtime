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

  printf("%s %s %d\n", binary1, binary2, length);
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
                            __global struct transition transitions[],
                            long long int current_state, char input[],
                            int length, __global char *output_ptr) {

  printf("%s\n", input);
  for (int i = 0; i < num_transitions; i++) {

    struct transition trans = transitions[i];
    //printf("%ld\n", trans.current_state);
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
                          int num_transitions,
                          int input_length,
                          int output_length) {

  int idx = get_global_id(0);
  struct partecl_input input_gen = inputs[idx];
  __global struct partecl_result *result_gen = &results[idx];
  result_gen->test_case_num = input_gen.test_case_num;

  char* input_ptr = input_gen.input_ptr;
  __global char* output_ptr = result_gen->output;

  long long int current_state = transitions[0].current_state;
  
  while (*input_ptr != '\0') {
    current_state = lookup_symbol(num_transitions, transitions, current_state,
                                  input_ptr, input_length, output_ptr);

    printf("Hello!\n");
    if (current_state == -1) {
      //return -1;
    }

      input_ptr += input_length;
    output_ptr += output_length;
  }

  //return current_state;
  result_gen->final_state = current_state;
}
