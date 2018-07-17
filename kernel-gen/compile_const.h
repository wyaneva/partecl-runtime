#ifndef COMPILE_CONST_H
#define COMPILE_CONST_H

/*****************************
 * Used in FSM structs "fsm.h"
 *****************************/

#ifndef INPUT_LENGTH_FSM
#define INPUT_LENGTH_FSM 2
#endif

#ifndef OUTPUT_LENGTH_FSM
#define OUTPUT_LENGTH_FSM 2
#endif

#ifndef NUM_STATES
#define NUM_STATES 42
#endif

#ifndef MAX_NUM_TRANSITIONS_PER_STATE
#define MAX_NUM_TRANSITIONS_PER_STATE 256
//2^(input_length)
//256 - for ASCII
#endif

/********************************/

#endif
