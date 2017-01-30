EXECUTABLES = run-in-parallel
FLAGS = -DGCC_COMPILED=1

all: $(EXECUTABLES)

run-in-parallel:  hello.c 
	gcc -std=gnu99 -o $@ $^ -lrt -lm -O3 $(FLAGS) -fopenmp

clean:
	rm -f $(EXECUTABLES) *.o
