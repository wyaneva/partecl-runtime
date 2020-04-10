## FSM Testing

This document relates to using ParTeCL-Runtime to execute FSM tests. 

  1. The FSM should be provided in the *kiss2* format.

  2. The file of the FSM should be provided with the **-filename** argument, eg.
     `./gpu-test 100 -filename some-fsm.kiss2`

  3. `source/constants.h` contains the following important compile-time constants:

     * For test layout:
       * `FSM_INPUTS_WITH_OFFSETS` - with-offsets
       * `FSM_INPUTS_COAL_CHAR` - padded-transposed
       * when all three are set to 0 - padded

     * For FSM storage:
       * `FSM_OPTIMISE_CONST_MEM` - store FSM in constant or local memory if possible
       * `FSM_CONSTANT_MEMORY` and `FSM_LOCAL_MEMORY` - used only in the kernel code, they are actually automatically set by the host at runtime, based on the value of `FSM_OPTIMISE_CONST_MEM` and the available memory space on the device.

  4. `kernel-gen/compile_const.h` contains additional compile-time constants, specific to the particular FSM being tested. They should be updated by ParTeCL-Codegen.
     * `INPUT_LENGTH_FSM` - number of input characters in each input in the test (`.i` in the kiss2 file) + 1 for `\0` character in the end.
     * `OUTPUT_LENGTH_FSM` - same, but for the output (`.0` in the kiss2 file) + 1 for `\0` character in the end.
     * `NUM_STATES` - number of states in the FSM
     * `MAX_NUM_TRANSITIONS_PER_STATE` - the number of all possible inputs for each state in the FSM

  5. `kernel-gen/structs.h` contains:
     * `PADDED_INPUT_ARRAY_SIZE` - ideally the length of the longest test case of the FSM, but it can be automatically generated to be arbitrarily big, some of the test layouts will dynamically allocate the right amount of memory.
