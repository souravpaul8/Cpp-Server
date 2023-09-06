CC = g++ -Wno-write-strings
GCC = gcc

SERVER_FILE = service_discovery_server.cpp

all: service_discovery_server

service_discovery_server: $(SERVER_FILE)
	$(CC) $(SERVER_FILE) -g -o service_discovery_server -std=c++11 -lcpprest -lboost_system -lcrypto -lssl -lboost_log -lboost_thread \
                      -lpthread
clean:
	rm -f service_discovery_server 