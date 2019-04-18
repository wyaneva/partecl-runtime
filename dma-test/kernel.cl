kernel void kernel_fun(global int *inputs, global int *results, int n) {

  int idx = get_global_id(0);

  if (idx < n) {

    //printf("%d\n", inputs[idx]);
    results[idx] = inputs[idx] * 100;
    //printf("%d\n", results[idx]);
  }
}
