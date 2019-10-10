EXECUTABLES = main

all: $(EXECUTABLES)

main:     main.c 
	clang -std=gnu99 -o$@ $^ -lm -g

clean:
	rm -f $(EXECUTABLES) *.o
