CC = g++ -Wno-write-strings
GCC = gcc

SERVER_FILE = redis_server.cpp

all: redis_server

redis_server: $(SERVER_FILE)
	$(CC) $(SERVER_FILE) -g -o redis_server -std=c++11 -lcpprest -lboost_system -lcrypto -lssl -lboost_log -lboost_thread \
                      -lpthread
clean:
	rm -f redis_server 
