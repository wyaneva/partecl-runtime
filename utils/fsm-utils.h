#ifndef FSM_UTILS_H
#define FSM_UTILS_H

#include <stdlib.h>

void calculate_sizes_with_offset(int *total_number_of_inputs,
                                 size_t *size_inputs_offset,
                                 const struct partecl_input *inputs,
                                 const int num_test_cases);

void partecl_input_to_input_with_offsets(const struct partecl_input *inputs,
                                         char *inputs_offset,
                                         int *offsets,
                                         const int num_test_cases);
 
void results_with_offsets_to_partecl_results(const char *results_offset,
                                             struct partecl_result *results,
                                             const int total_number_of_inputs,
                                             const int *offsets,
                                             const int num_test_cases);

int sort_test_cases_by_length(struct partecl_input *inputs, int num_test_cases);

#endif
