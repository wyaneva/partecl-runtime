#include <string.h>
#include <stdlib.h>
#include "../kernel-gen/structs.h"

void calculate_sizes_with_offset(int *total_number_of_inputs,
                                 size_t *size_inputs_offset,
                                 const struct partecl_input *inputs,
                                 const int num_test_cases) {

  // calculate how much space we need for the test case inputs
  *total_number_of_inputs = 0;
  for (int i = 0; i < num_test_cases; i++) {
    *total_number_of_inputs += strlen(inputs[i].input_ptr) + 1;
  }
  *size_inputs_offset = sizeof(char) * (*total_number_of_inputs);
}

void partecl_input_to_input_with_offsets(const struct partecl_input *inputs,
                                         char *inputs_offset,
                                         int *offsets,
                                         const int num_test_cases) {
  // move inputs into one big array
  char *inputsptr = inputs_offset;
  for (int i = 0; i < num_test_cases; i++) {
    int length = strlen(inputs[i].input_ptr);
    strcpy(inputsptr, inputs[i].input_ptr);
    inputsptr += length;
    *inputsptr = '\0';
    inputsptr++;

    // calculate offset
    offsets[i] =
        i == 0 ? 0 : offsets[i - 1] + strlen(inputs[i - 1].input_ptr) + 1;
  }
}

void results_with_offsets_to_partecl_results(const char *results_offset,
                                             struct partecl_result *results,
                                             const int total_number_of_inputs,
                                             const int *offsets,
                                             const int num_test_cases) {
  for (int i = 0; i < num_test_cases; i++) {
    char *outputptr = &results[i].output[0];
    int start = offsets[i];
    int end = i == num_test_cases - 1
                  ? start + total_number_of_inputs - offsets[i]
                  : start + offsets[i + 1] - offsets[i];
    for (int j = start; j < end; j++) {
      *outputptr = results_offset[j];
      outputptr++;
    }
    *outputptr = '\0';
    results[i].length = end - start - 1;
  }
}