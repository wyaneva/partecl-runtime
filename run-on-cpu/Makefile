EXECUTABLES = run-on-cpu
FLAGS = -DGCC_COMPILED=1

all: $(EXECUTABLES)

run-on-cpu:  run-on-cpu.c \
	     ../utils/options.c \
	     ../utils/read-test-cases.c \
	     ../utils/utils.c \
	     ../kernel-gen/cpu-gen.c \
	     gen/test-cl.c
	gcc -std=gnu99 -o $@ $^ -lrt -lm -O3 $(FLAGS)

clean:
	rm -f $(EXECUTABLES) *.o
