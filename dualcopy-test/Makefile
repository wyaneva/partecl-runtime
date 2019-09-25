EXECUTABLES = main

all: $(EXECUTABLES)

  CL_CFLAGS = -I/opt/cuda/include
  CL_LDFLAGS = -L/opt/cuda/lib

main:     main.c \
	  ../source/cl-utils.c \
	  ../utils/utils.c
	clang $(CL_CFLAGS) $(CL_LDFLAGS) -std=gnu99 -o$@ $^ -lrt -lOpenCL $(GPU_FLAGS) -lm -g

clean:
	rm -f $(EXECUTABLES) *.o
