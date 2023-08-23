CC = g++ -Wno-write-strings
GCC = gcc

LOAD_GEN_FILE = load_gen.c

all: load_gen

load_gen: $(LOAD_GEN_FILE)
	$(GCC) $(LOAD_GEN_FILE) -pthread -o load_gen 

clean:
	rm -f load_gen 
