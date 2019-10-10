#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IS_TRANSPOSED 1

//#define TESTS 8388608
#define TESTS 10
//#define SIZE 438
#define SIZE 5

void transpose_inputs_char(char *inputs, int max_input_size, int num_test_cases); 
void transpose_inputs_char_new(char *inputs, int max_input_size, int num_test_cases); 
void print_inputs(char *inputs, int max_input_size, int num_test_cases); 

int main(int argc, const char **argv) {

  int num_test_cases = TESTS;
  int max_input_size = SIZE;

  size_t size = sizeof(char) * max_input_size * num_test_cases;
  char *inputs = (char*)malloc(size);

  char* inputptr = inputs;
#if IS_TRANSPOSED
  for(int j = 0; j < max_input_size; j++) {
    for(int i = 0; i < num_test_cases; i++) {
#else
  for(int i = 0; i < num_test_cases; i++) {
    for(int j = 0; j < max_input_size; j++) {
#endif
      *inputptr = i+'0';
      inputptr++;
    }
  }

#if IS_TRANSPOSED
  print_inputs(inputs, num_test_cases, max_input_size);
#else
  print_inputs(inputs, max_input_size, num_test_cases);
#endif
}

void transpose_inputs_char_new(char *inputs, int max_input_size, int num_test_cases) {
}

void transpose_inputs_char(char *inputs, int max_input_size, int num_test_cases) {
  // transpose inputs for coalesced reading on gpu
  size_t size = sizeof(char) * max_input_size * num_test_cases;
  char *inputs_temp = (char *)malloc(size);

  for (int i = 0; i < num_test_cases; i++) {
    char *inputs_temp_ptr = inputs_temp + i;
    char *inputs_ptr = inputs + i * max_input_size;
    for (int j = 0; j < max_input_size; j++) {
      *inputs_temp_ptr = *inputs_ptr;
      inputs_temp_ptr += num_test_cases;
      inputs_ptr++;
    }
  }

  char *inputs_ptr = inputs;
  char *inputs_temp_ptr = inputs_temp;
  for (int i = 0; i < num_test_cases * max_input_size; i++) {
    *inputs_ptr = *inputs_temp_ptr;
    inputs_ptr++;
    inputs_temp_ptr++;
  }

  free(inputs_temp);
}

void print_inputs(char *inputs, int max_input_size, int num_test_cases) {

  char* inputptr = inputs;
  for(int i = 0; i < num_test_cases; i++) {
    for(int j = 0; j < max_input_size; j++) {
      printf("%c ", *inputptr);
      inputptr++;
    }
    printf("\n");
  }
}
