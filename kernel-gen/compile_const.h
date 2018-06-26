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
#define NUM_STATES 43
#endif

#ifndef MAX_NUM_TRANSITIONS_PER_STATE
#define MAX_NUM_TRANSITIONS_PER_STATE 255
//8 //255 - for ASCII
#endif

/********************************/

/*****************************
 * Used in kernel
 *****************************/

#ifndef NUM_TRANSITIONS_KERNEL
#define NUM_TRANSITIONS_KERNEL 5331
#endif

#ifndef OUTPUT_LENGTH_KERNEL
#define OUTPUT_LENGTH_KERNEL 300
#endif

/********************************/

#endif
