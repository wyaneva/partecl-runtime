EXECUTABLES = cuda-test
OBJECTS = kernel.o 

CL_CFLAGS = -I/opt/cuda-8.0.44/include
CL_LDFLAGS = -L/opt/cuda-8.0.44/lib64

all: $(EXECUTABLES)

#compile cuda - device
kernel.o: kernel.cu 
	nvcc $^ -o $@ -arch=sm_20 -c

#compile c - host
cuda-test: runtime.c \
 	  ../utils/options.c \
          ../utils/read-test-cases.c \
 	  ../utils/utils.c \
 	  ../kernel-gen/cpu-gen.c \
	  $(OBJECTS)
	clang $^ -o $@ $(CL_CFLAGS) $(CL_LDFLAGS) -lrt -lcuda -lcudart -std=gnu99

clean:
	rm -f $(EXECUTABLES) *.o
