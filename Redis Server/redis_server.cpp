#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include "redis_repository.h"
#include <boost/algorithm/string.hpp>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

const utility::string_t SERVER_URL = U("http://127.0.0.1:5005");

void getTimestamp(char *timestamp, int timestampSize) {
    struct timeval tv;
    struct tm *tm_info;

    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);

    snprintf(timestamp, timestampSize, "%02d:%02d:%02d.%06ld", 
        tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, tv.tv_usec);
}

void handle_get(const http_request& request)
{
    char timestampStart[20]; // Adjust the size as needed
    getTimestamp(timestampStart, sizeof(timestampStart));

    utility::string_t path = request.request_uri().path();

    std::vector<utility::string_t> segments;
    boost::split(segments, path, boost::is_any_of("/"), boost::token_compress_on);

    std::string token;
    if (segments.size() > 1) {
        token = segments[1];
    }

    redis_data_repository repository;
    try{
        repository.storeData(token);
    } catch (std::exception &e) {
        std::cout << "Redis Store: EXCEPTION CAUGHT: " << e.what() << std::endl;
    }
    web::json::value response;
    try {
        response = repository.getData(token);
    } catch (std::exception &e) {
        std::cout << "Redis Get: EXCEPTION CAUGHT: " << e.what() << std::endl;
    }
    
    utility::string_t responseString;
    try{
        responseString = response.serialize();
    } catch (std::exception &e) {
        std::cout << "Redis Serialize: EXCEPTION CAUGHT: " << e.what() << std::endl;
    }

    http_response httpResponse(status_codes::OK);
    httpResponse.headers().set_content_type(U("application/json"));
    httpResponse.set_body(response);

    char timestampEnd[20];
    getTimestamp(timestampEnd, sizeof(timestampEnd));
    httpResponse.headers().add(U("redis_begin_time"),U(timestampStart));
    httpResponse.headers().add(U("redis_end_time"),U(timestampEnd));

    request.reply(httpResponse);
    
    //std::cout << "Connected" << std::endl;
}

int main()
{
    http_listener listener(SERVER_URL);

    listener.support(methods::GET, handle_get);

    try {
        listener.open().then([&listener]() {
            std::cout << "Server started listening on " << SERVER_URL << std::endl;
        }).wait();

        std::cout << "Press Enter to exit." << std::endl;
        std::cin.ignore();

        listener.close().wait();
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    return 0;
}
