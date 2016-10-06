EXECUTABLES = test-on-gpu

all: $(EXECUTABLES)

  CL_CFLAGS = -I/opt/cuda/include
  CL_LDFLAGS = -L/opt/cuda/lib

test-on-gpu: ../source/runtime.c \
	     ../source/cl-utils.c \
	     ../utils/options.c \
             ../utils/read-test-cases.c \
	     ../utils/utils.c \
	     ../kernel-gen/cpu-gen.c 
	gcc $(CL_CFLAGS) $(CL_LDFLAGS) -std=gnu99 -o$@ $^ -lrt -lOpenCL

clean:
	rm -f $(EXECUTABLES) *.o
