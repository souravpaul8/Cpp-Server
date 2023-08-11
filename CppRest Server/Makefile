CC = g++ -Wno-write-strings
GCC = gcc

SERVER_FILE = rest_server.cpp
HTTP_SERVER_FILE = http_server.cpp
LOAD_GEN_FILE = load_gen.c

all: rest_server load_gen

rest_server: $(SERVER_FILE) $(HTTP_SERVER_FILE)
	$(CC) $(SERVER_FILE) $(HTTP_SERVER_FILE) -g -o rest_server -lpthread -lcpprest -lcrypto

load_gen: $(LOAD_GEN_FILE)
	$(GCC) $(LOAD_GEN_FILE) -pthread -o load_gen 

clean:
	rm -f rest_server load_gen 
