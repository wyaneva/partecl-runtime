#ifndef FSM_H
#define FSM_H

#include "fsm_const.h"

typedef struct {
  short next_state;
  char output[OUTPUT_LENGTH_FSM];
} transition;

enum fsm_parameter { INPUT_LENGTH, OUTPUT_LENGTH, NUM_TRANSITIONS, STATE_BASE };

/**
 * Read an FSM from a KISS2 file.
 * Defined in fsm.c
 */
transition* read_fsm(const char *filename, int *num_transitions,
                     int *starting_state, int *input_length,
                     int *output_length);

int get_index(short current_state, char input);

#endif
