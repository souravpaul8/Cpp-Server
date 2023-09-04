using namespace std;
#include <iostream>
#include "redis_repository.h"

const char *hostname = "127.0.0.1";
int port = 6379;

// void redis_data_repository::connect_To_Database(redisContext **c)
// {
//     const char *hostname = "127.0.0.1";
//     int port = 6379;

//     *c = redisConnect(hostname,port);
    
//     struct timeval timeout = { 1, 500000 }; // 1.5 seconds
//     *c = redisConnectWithTimeout(hostname, port, timeout);
//     if (*c == NULL || (*c)->err) {
//         if (*c) {
//             printf("Connection error: %s\n", (*c)->errstr);
//             redisFree(*c);
//         } else {
//             printf("Connection error: can't allocate redis context\n");
//         }
//         exit(1);
//     }
//     //printf("Connected to redis\n");
//     return;
// }

web::json::value redis_data_repository::getData() {
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    redisContext *c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || (c)->err) {
        if (c) {
            printf("Connection error: %s\n", (c)->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }
    //connect_To_Database(&c);
    
    redisReply *reply;
    reply = (redisReply *)redisCommand(c,"GET %s","foo");
    web::json::value temp1;
    temp1[U("Message")] = web::json::value::string(reply->str);
    // printf("GET %s \t\t| ","foo");
    // printf("%s\n",reply->str);
    freeReplyObject(reply);
    redisFree(c);
    return temp1;
}

void redis_data_repository::storeData() {
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    redisContext *c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || (c)->err) {
        if (c) {
            printf("Connection error: %s\n", (c)->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }
    redisReply *reply;
    reply = (redisReply *)redisCommand(c,"SET %s %s", "foo", "hello world");
    //printf("SET %s %s \t| %s\n", "foo", "hello world", reply->str);
    freeReplyObject(reply);
    redisFree(c);
    return;
}