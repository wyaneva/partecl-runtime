#ifndef FSM_H
#define FSM_H

enum fsm_parameter { INPUT_LENGTH, OUTPUT_LENGTH, NUM_TRANSITIONS, STATE_BASE };

struct transition {
  char input[7];
  short current_state;
  short next_state;
  char output[7];
};

/**
 * Read an FSM from a KISS2 file.
 * Defined in fsm.c
 */
struct transition *read_fsm(const char *filename, int *num_transitions,
                            int *input_length, int *output_length);

#endif
