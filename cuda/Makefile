EXECUTABLES = cuda-test 
OBJECTS = options.o read-test-cases.o utils.o cpu-gen.o

GPU_FLAGS = -DKERNEL_OPTIONS="\"-I $(HOME)/partecl-runtime/kernel-gen/ -I $(HOME)/clClibc/\""
CPU_FLAGS = -DGCC_COMPILED=1


  CL_CFLAGS = -I/opt/cuda/include
  CL_LDFLAGS = -L/opt/cuda/lib

all: $(objects)
	clang $(CL_CFLAGS) $(CL_LDFLAGS) ../utils/options.c ../utils/read-test-cases.c ../utils/utils.c ../kernel-gen/cpu-gen.c 

cuda-test: 
	nvcc runtime.cu 
clean:
	rm -f $(EXECUTABLES) *.o
