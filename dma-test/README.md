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

clCreateBuffer(buf_inputs);
clCreateBuffer(buf_results);

//TODO: assign kernel arguments

int *inputs_dma = clEnqueueMapBuffer(buf_inputs, CL_MAP_WRITE);
memcpy(inputs_dma, inputs)
clEnqueueUnmapMemObject(buf_inputs, inputs_dma);

clEnqueueNDRangeKernel();

int *results_dma = clEnqueueMapBuffer(buf_results, CL_MAP_READ);
memcpy(results, results_dma)
clEnqueueUnmapMemObject(buf_results, results_dma);

//TODO: use results

```

