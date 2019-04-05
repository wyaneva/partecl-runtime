kernel void kernel_fun(global int *inputs, global int *results, int n) {

  int idx = get_global_id(0);

  if (idx < n) {

    results[idx] = inputs[idx] * 100;
    //printf("%d\n", results[idx]);
  }
}
