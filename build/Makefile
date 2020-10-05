EXECUTABLES = gpu-test gpu-test-debug cpu-test cpu-test-debug

GPU_FLAGS = -DKERNEL_OPTIONS="\"-I $(shell pwd)/../kernel-gen/ -I $(HOME)/clClibc/ -DGPU_COMPILED=1 -DBENCHMARK=1 -DINPUT_IMG=1 \""
CPU_FLAGS = -I $(HOME)/clClibc/ -DGCC_COMPILED=1 -DBENCHMARK=1 -DINPUT_IMG=1

all: $(EXECUTABLES)

  CL_CFLAGS = -I/opt/cuda/include
  CL_LDFLAGS = -L/opt/cuda/lib

gpu-test: ../source/runtime.c \
	  ../source/cl-utils.c \
	  ../utils/options.c \
          ../utils/read-test-cases.c \
	  ../utils/utils.c \
	  ../kernel-gen/cpu-gen.c 
	clang $(CL_CFLAGS) $(CL_LDFLAGS) -std=gnu99 -o$@ $^ -lrt -lOpenCL $(GPU_FLAGS) $(CPU_FLAGS)

gpu-test-debug: ../source/runtime.c \
	  ../source/cl-utils.c \
	  ../utils/options.c \
          ../utils/read-test-cases.c \
	  ../utils/utils.c \
	  ../kernel-gen/cpu-gen.c
	clang $(CL_CFLAGS) $(CL_LDFLAGS) -std=gnu99 -o$@ $^ -lrt -lOpenCL $(GPU_FLAGS) $(CPU_FLAGS) -g

cpu-test: ../run-on-cpu/run-on-cpu.c \
	  ../utils/options.c \
	  ../utils/read-test-cases.c \
	  ../utils/utils.c \
	  ../kernel-gen/cpu-gen.c \
	  ../kernel-gen/main-cl.c
	gcc -std=gnu99 -o $@ $^ -lrt -lm -O3 $(CPU_FLAGS) -fopenmp -Wno-implicit-function-declaration 

cpu-test-debug: ../run-on-cpu/run-on-cpu.c \
	  ../utils/options.c \
	  ../utils/read-test-cases.c \
	  ../utils/utils.c \
	  ../kernel-gen/cpu-gen.c \
	  ../kernel-gen/main-cl.c
	gcc -std=gnu99 -o $@ $^ -lrt -lm -O3 $(CPU_FLAGS) -fopenmp -Wno-implicit-function-declaration -g

clean:
	rm -f $(EXECUTABLES) *.o
