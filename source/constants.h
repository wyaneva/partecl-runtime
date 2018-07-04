#ifndef CONSTANTS_H
#define CONSTANTS_H

/**********************************
 * Used in kernel for optimisations
 **********************************/

/*
 * BMRK toggles special code for C programs or FSM network and open source benchmarks
 */
#ifndef BMRK_C
#define BMRK_C 0
#endif

#ifndef BMRK_NETWORK
#define BMRK_NETWORK 1
#endif

#if BMRK_NETWORK
#define BMRK_OS 0
#else
#define BMRK_OS 1
#endif

/*
 * Input data layout toggles
 */

// FSM_INPUTS_WITH_OFFSETS toggles dense input representation
#ifndef FSM_INPUTS_WITH_OFFSETS
#define FSM_INPUTS_WITH_OFFSETS 0
#endif

//  FSM_INPUTS toggles transposing data for memory coalescing
//  1. FSM_INPUTS_COAL_CHAR
//  2. FSM_INPUTS_COAL_CHAR4
//  They can ONLY be on when FSM_INPUTS_WITH_OFFSETS is NOT on

#ifndef FSM_INPUTS_COAL_CHAR
#define FSM_INPUTS_COAL_CHAR 0
#endif

#ifndef FSM_INPUTS_COAL_CHAR4
#define FSM_INPUTS_COAL_CHAR4 1
#endif

#if FSM_INPUTS_WITH_OFFSETS || FSM_INPUTS_COAL_CHAR || FSM_INPUTS_COAL_CHAR4
#define TEST_INPUTS_ATTR global
#else
#define TEST_INPUTS_ATTR
#endif

#if FSM_INPUTS_COAL_CHAR4
#define CHAR_N 4
#define TEST_INPUTS_TYPE char4
#else
#define TEST_INPUTS_TYPE char
#endif

/*
 * FSM_OPTIMISE_CONST toggles optimisations
 *  1. FSM storage in constant & local memory where possible
 */
#ifndef FSM_OPTIMISE_CONST_MEM
#define FSM_OPTIMISE_CONST_MEM 1
#endif

/*
 * FSM_CONSTANT_MEMORY and FSM_LOCAL_MEMORY
 * put the FSM in the corresponding memory spaces
 * when there is enough space
 */
#ifndef FSM_CONSTANT_MEMORY
#define FSM_CONSTANT_MEMORY 0
#endif

#ifndef FSM_LOCAL_MEMORY
#define FSM_LOCAL_MEMORY 0
#endif

#if FSM_CONSTANT_MEMORY
#define FSM_ATTR constant
#define FSM_ATTR_KNL constant
#else

#if FSM_LOCAL_MEMORY
#define FSM_ATTR local
#define FSM_ATTR_KNL global
#else
#define FSM_ATTR global
#define FSM_ATTR_KNL global
#endif

#endif

#endif
