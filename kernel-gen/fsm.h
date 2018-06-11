#ifndef FSM_H
#define FSM_H

#include "compile_const.h"

enum fsm_parameter { INPUT_LENGTH, OUTPUT_LENGTH, NUM_TRANSITIONS, STATE_BASE };

struct transition {
  char input[INPUT_LENGTH_FSM];
  short current_state;
  short next_state;
  char output[INPUT_LENGTH_FSM];
};

/**
 * Read an FSM from a KISS2 file.
 * Defined in fsm.c
 */
struct transition *read_fsm(const char *filename, int *num_transitions,
                            int *input_length, int *output_length);

#endif
