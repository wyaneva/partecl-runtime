A toy program, which profiles data transfer with/without DMA transfer.


#### Without DMA transfer

```
int *inputs = malloc;
int *results = malloc;

//TODO: populate inputs

clCreateBuffer(buf_inputs);
clCreateBuffer(buf_results);

//TODO: assign kernel arguments

clEnqueueWriteBuffer(buf_inputs, inputs);

clEnqueueNDRangeKernel();

clEnqueueReadBuffer(buf_results, results);

//TODO: use results

```


#### With DMA transfer

```
int *inputs = malloc;
int *results = malloc;

//TODO: populate inputs

// this allows the kernel to use data allocated on the host
clCreateBuffer(buf_inputs, CL_MEM_USE_HOSR_PTR, inputs);
clCreateBuffer(buf_results, CL_MEM_USE_HOST_PTR, results);

//TODO: assign kernel arguments

// input data is implicitly copied to device memory
clEnqueueNDRangeKernel();

// this transfers the results to host memory
int *results_dma = clEnqueueMapBuffer(buf_results, CL_MAP_READ);
clEnqueueUnmapMemObject(buf_results, results_dma);

//TODO: use results

```

