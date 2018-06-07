#ifndef STRUCTS_H
#define STRUCTS_H

#define PADDED_INPUT_ARRAY_SIZE 500

typedef struct partecl_input
{
  int test_case_num;
  int argc;
  char input_ptr[PADDED_INPUT_ARRAY_SIZE];
} partecl_input;

typedef struct partecl_result
{
  int test_case_num;
  int final_state;
  char output[PADDED_INPUT_ARRAY_SIZE];
  int length;
} partecl_result;

#endif
