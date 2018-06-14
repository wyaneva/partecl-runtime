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

/**********************************
 * Used in kernel for optimisations
 **********************************/

/*
 * FSM_OPTIMISE toggles optimisations
 *  1. coalesced memory allocation
 *  2. hash table storage
 */
#ifndef FSM_OPTIMISE
#define FSM_OPTIMISE 0
#endif

#if FSM_OPTIMISE
#define TEST_INPUTS_ATTR global
#else
#define TEST_INPUTS_ATTR
#endif

/*
 * FSM_LOCAL_MEMORY puts the FSM in local memory
 * when there is enough space
 */
#ifndef FSM_LOCAL_MEMORY
#define FSM_LOCAL_MEMORY 0
#endif

#if FSM_LOCAL_MEMORY
#define TRANSITIONS_ATTR local
#else
#define TRANSITIONS_ATTR global
#endif

#endif
