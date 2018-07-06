#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../kernel-gen/structs.h"
#include "../source/constants.h"
#include "../utils/utils.h"

void print_sanity_checks() {
  printf("SANITY CHECK!");
  printf("\n");
#if FSM_INPUTS_WITH_OFFSETS
  printf("FSM_INPUTS_WITH_OFFSETS = %d\n", FSM_INPUTS_WITH_OFFSETS);
#else
#if !FSM_INPUTS_WITH_OFFSETS
  printf("FSM_INPUTS_WITH_OFFSETS = %d\n", FSM_INPUTS_WITH_OFFSETS);
#else
  printf("ERROR! FSM_INPUTS_WITH_OFFSETS not set.\n");
#endif
#endif

#if FSM_INPUTS_COAL_CHAR
  printf("FSM_INPUTS_COAL_CHAR = %d\n", FSM_INPUTS_COAL_CHAR);
#else
#if !FSM_INPUTS_COAL_CHAR
  printf("FSM_INPUTS_COAL_CHAR = %d\n", FSM_INPUTS_COAL_CHAR);
#else
  printf("ERROR! FSM_INPUTS_COAL_CHAR not set.\n");
#endif
#endif

#if FSM_INPUTS_COAL_CHAR4
  printf("FSM_INPUTS_COAL_CHAR4 = %d\n", FSM_INPUTS_COAL_CHAR4);
#else
#if !FSM_INPUTS_COAL_CHAR4
  printf("FSM_INPUTS_COAL_CHAR4 = %d\n", FSM_INPUTS_COAL_CHAR4);
#else
  printf("ERROR! FSM_INPUTS_COAL_CHAR4 not set.\n");
#endif
#endif

#if FSM_INPUTS_COAL_CHAR && FSM_INPUTS_COAL_CHAR4
  printf("ERROR! FSM_INPUTS_COAL_CHAR and FSM_INPUTS_COAL_CHAR4 cannot both be set.\n");
#endif

#if FSM_INPUTS_WITH_OFFSETS && FSM_INPUTS_COAL_CHAR
  printf("ERROR! FSM_INPUTS_WITH_OFFSETS and FSM_INPUTS_COAL_CHAR cannot both be set.\n");
#endif

#if FSM_INPUTS_WITH_OFFSETS && FSM_INPUTS_COAL_CHAR4
  printf("ERROR! FSM_INPUTS_WITH_OFFSETS and FSM_INPUTS_COAL_CHAR4 cannot both be set.\n");
#endif
  printf("\n");
}

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
  }
}

static int test_case_length(struct partecl_input input) {
  return strlen(input.input_ptr);
}

static int min(struct partecl_input x, struct partecl_input y) {
  int xl = test_case_length(x);
  int yl = test_case_length(y);

  if (xl < yl) {
    return xl;
  } else {
    return yl;
  }
}

static void merge_helper(struct partecl_input *inputs, int left, int right,
                         struct partecl_input *temp) {
  /* base case: one element */
  if (right == left + 1) {
    return;
  } else {
    int i = 0;
    int length = right - left;
    int midpoint_distance = length / 2;
    /* l and r are to the positions in the left and right subarrays */
    int l = left, r = left + midpoint_distance;

    /* sort each subarray */
    merge_helper(inputs, left, left + midpoint_distance, temp);
    merge_helper(inputs, left + midpoint_distance, right, temp);

    /* merge the arrays together using scratch for temporary storage */
    for (i = 0; i < length; i++) {
      /* Check to see if any elements remain in the left array; if so,
       * we check if there are any elements left in the right array; if
       * so, we compare them.  Otherwise, we know that the merge must
       * use take the element from the left array */
      if (l < left + midpoint_distance &&
          (r == right ||
           min(inputs[l], inputs[r]) == test_case_length(inputs[l]))) {
        temp[i] = inputs[l];
        l++;
      } else {
        temp[i] = inputs[r];
        r++;
      }
    }
    /* Copy the sorted subarray back to the input */
    for (i = left; i < right; i++) {
      inputs[i] = temp[i - left];
    }
  }
}

int sort_test_cases_by_length(struct partecl_input *inputs,
                              int num_test_cases) {
  struct partecl_input *temp = (struct partecl_input *)malloc(
      sizeof(struct partecl_input) * num_test_cases);
  if (temp != NULL) {
    merge_helper(inputs, 0, num_test_cases, temp);
    free(temp);
    return SUCCESS;
  } else {
    return FAIL;
  }
}
