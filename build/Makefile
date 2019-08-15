EXECUTABLES = gpu-test cpu-test gpu-test-debug

GPU_FLAGS = -DKERNEL_OPTIONS="\"-I $(HOME)/partecl-runtime/kernel-gen/ -I $(HOME)/partecl-runtime/source -I $(HOME)/clClibc/ -DGPU_COMPILED=1\""

CPU_FLAGS = -DGCC_COMPILED=1 -DTEST_DIST_FILE="\"$(HOME)/fsmtesting/2019journal/data/test-analysis/test-suite-size.analysis\""

all: $(EXECUTABLES)

  CL_CFLAGS = -I/opt/cuda/include
  CL_LDFLAGS = -L/opt/cuda/lib

gpu-test: ../source/runtime.c \
	  ../source/cl-utils.c \
	  ../utils/options.c \
          ../utils/read-test-cases.c \
	  ../utils/utils.c \
	  ../utils/fsm-utils.c \
	  ../kernel-gen/cpu-gen.c 
	clang $(CL_CFLAGS) $(CL_LDFLAGS) -std=gnu99 -o$@ $^ -lrt -lOpenCL $(GPU_FLAGS) -lm

cpu-test: ../run-on-cpu/run-on-cpu.c \
	  ../utils/options.c \
	  ../utils/read-test-cases.c \
	  ../utils/utils.c \
	  ../utils/fsm-utils.c \
	  ../kernel-gen/cpu-gen.c \
	  ../cpu-gen/test-cl.c
	gcc -std=gnu99 -o $@ $^ -lrt -lm -O3 $(CPU_FLAGS) -fopenmp -Wno-implicit-function-declaration -lm

gpu-test-debug: ../source/runtime.c \
	  ../source/cl-utils.c \
	  ../utils/options.c \
          ../utils/read-test-cases.c \
	  ../utils/utils.c \
	  ../utils/fsm-utils.c \
	  ../kernel-gen/cpu-gen.c 
	clang $(CL_CFLAGS) $(CL_LDFLAGS) -std=gnu99 -o$@ $^ -lrt -lOpenCL $(GPU_FLAGS) -lm -g


clean:
	rm -f $(EXECUTABLES) *.o
