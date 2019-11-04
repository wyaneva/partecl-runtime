#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cpu-gen.h"

void populate_inputs(struct partecl_input *input, int argc, char** args, int stdinc, char** stdins)
{
  if(argc >= 2)
  {
    char *input_ptr_ptr = args[1];
    int idx = 0;
    while(*input_ptr_ptr != '\0')
    {
      input->input_ptr[idx] = *input_ptr_ptr;
      input_ptr_ptr++;
      idx++;
    }
    for(int i = idx; i < PADDED_INPUT_ARRAY_SIZE; i++)
    {
    input->input_ptr[i] = '\0';
    }
  }
}
void compare_results(struct partecl_result* results, struct partecl_result* exp_results, int num_test_cases)
{
  for(int i = 0; i < num_test_cases; i++)
  {
    struct partecl_result curres = results[i];
    printf("TC %d: ", i+1);
    char* outputptr = curres.output;
    while(*outputptr != '\0')
    {
      printf("%c ", *outputptr);
      outputptr++;
    }
    printf("\n");
  }
}

void compare_results_char(char* results[], char* exp_results[], int num_test_cases)
{
  for(int i = 0; i < num_test_cases; i++)
  {
    char* outputptr = results[i];
    printf("TC %d: ", i+1);
    while(*outputptr != '\0')
    {
      printf("%c ", *outputptr);
      outputptr++;
    }
    printf("\n");
  }
}
