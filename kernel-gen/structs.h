#ifndef STRUCTS_H
#define STRUCTS_H

#define PADDED_INPUT_ARRAY_SIZE 15

typedef struct partecl_input
{
  char input_ptr[PADDED_INPUT_ARRAY_SIZE];
} partecl_input;

typedef struct partecl_result
{
  char output[PADDED_INPUT_ARRAY_SIZE];
} partecl_result;

#endif
