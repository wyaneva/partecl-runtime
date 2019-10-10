kernel void kernel_fun(global int *inputs, global int *results, int n) {

  int idx = get_global_id(0);

  if (idx < n) {

    for(int i = 0; i < 10; i++) {
      results[idx] = inputs[idx] + 1;
    }
  }
}
