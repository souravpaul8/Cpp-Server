CC = g++ -Wno-write-strings
GCC = gcc

SERVER_FILE = redis_socket_server.cpp
HTTP_SERVER_FILE = http_server.cpp

all: redis_server load_gen

redis_server: $(SERVER_FILE) $(HTTP_SERVER_FILE)
	$(CC) $(SERVER_FILE) $(HTTP_SERVER_FILE) -g -o redis_server -lpthread

clean:
	rm -f redis_server
