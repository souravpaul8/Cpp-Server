CC = g++ -Wno-write-strings
GCC = gcc

SERVER_FILE = tcp_server.cpp
HTTP_SERVER_FILE = http_server.cpp
LOAD_GEN_FILE = load_gen.c

all: restserver load_gen

restserver: $(SERVER_FILE) $(HTTP_SERVER_FILE)
	$(CC) $(SERVER_FILE) $(HTTP_SERVER_FILE) -g -o restserver -lpthread -lcpprest -lcrypto

load_gen: $(LOAD_GEN_FILE)
	$(GCC) $(LOAD_GEN_FILE) -pthread -o load_gen 

clean:
	rm -f restserver load_gen 
