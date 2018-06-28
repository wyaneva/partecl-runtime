#ifndef COMPILE_CONST_H
#define COMPILE_CONST_H

/*****************************
 * Used in FSM structs "fsm.h"
 *****************************/

#ifndef INPUT_LENGTH_FSM
#define INPUT_LENGTH_FSM 8
#endif

#ifndef OUTPUT_LENGTH_FSM
#define OUTPUT_LENGTH_FSM 8
#endif

#ifndef NUM_STATES
#define NUM_STATES 16
#endif

#ifndef MAX_NUM_TRANSITIONS_PER_STATE
#define MAX_NUM_TRANSITIONS_PER_STATE 128
//2^(input_length)
//256 - for ASCII
#endif

/********************************/

/*****************************
 * Used in kernel
 *****************************/

#ifndef OUTPUT_LENGTH_KERNEL
#define OUTPUT_LENGTH_KERNEL 300
#endif

/********************************/

#endif
