#ifndef FSM_H
#define FSM_H

#include "compile_const.h"

typedef struct {
  char input[INPUT_LENGTH_FSM];
  short next_state;
  char output[OUTPUT_LENGTH_FSM];
} transition;

enum fsm_parameter { INPUT_LENGTH, OUTPUT_LENGTH, NUM_TRANSITIONS, STATE_BASE };

/**
 * Read an FSM from a KISS2 file.
 * Defined in fsm.c
 */
transition *read_fsm(const char *filename, int *num_transitions_per_state,
                     int *num_transitions, int *starting_state,
                     int *input_length, int *output_length);

#endif
