#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include "cpprest/uri.h"
#include <chrono>
#include <boost/algorithm/string.hpp>

#include "http_server.hh"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

const int timeoutSeconds = 5;
FILE *log_file;


const utility::string_t SERVER_URL = U("http://localhost:80");
const utility::string_t SERVICE_DISCOVERY_SERVER_URL = U("http://localhost:5001");

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
    
    utility::string_t token = segments[1];
    utility::string_t link;
    for(int i = 2;i<segments.size();i++){
        link += "/" + segments[i];
    }
    
    //std::cout << link << std::endl;


    http::uri serviceDiscoveryUri = http::uri(U("http://localhost:5001"));
    http::client::http_client serviceDiscovery(http::uri_builder(serviceDiscoveryUri)
                        .append_path(U("/redis/"))
                        .append_path(U(token))
                        .to_uri());

    char timestampDiscoveryRequestStart[20]; // Adjust the size as needed
    getTimestamp(timestampDiscoveryRequestStart, sizeof(timestampDiscoveryRequestStart));

    serviceDiscovery.request(methods::GET)
    .then([=](pplx::task<http_response> task)
    {
        http_response resp;
        try {
            resp = task.get();
        }catch(std::exception &e) {
            std::cout << "ServiceDiscovery: EXCEPTION CAUGHT: " << e.what() << std::endl;
        }
        char timestampDiscoveryRequestEnd[20]; // Adjust the size as needed
        getTimestamp(timestampDiscoveryRequestEnd, sizeof(timestampDiscoveryRequestEnd));
        string discovery_begin_time = resp.headers()["discovery_begin_time"];
        string discovery_end_time = resp.headers()["discovery_end_time"];

        resp.extract_json()
            .then([=] (pplx::task<json::value> task)
            {
                json::value regResp;
                string message;
                string discoveryToken;
                try {
                    regResp = task.get();
                } catch (std::exception& e) {
                    std::cout << "ServiceDiscovery: EXCEPTION CAUGHT: " << e.what() << std::endl;
                }

                message = regResp["message"].as_string();
                discoveryToken = regResp["tokenValue"].as_string();

                http::uri redisServerUri = http::uri(U(message));

                http::client::http_client redis(http::uri_builder(redisServerUri)
                                    .append_path(U("/"))
                                    .append_path(U(discoveryToken))
                                    .to_uri());
                
                char timestampRedisRequestStart[20]; // Adjust the size as needed
                getTimestamp(timestampRedisRequestStart, sizeof(timestampRedisRequestStart));
                redis.request(methods::GET)
                .then([=](pplx::task<http_response> task)
                {
                    http_response resp;
                    try {
                        resp = task.get();
                    }catch(std::exception &e) {
                        std::cout << "Redis: EXCEPTION CAUGHT: " << e.what() << std::endl;
                    }
                    char timestampRedisRequestEnd[20]; // Adjust the size as needed
                    getTimestamp(timestampRedisRequestEnd, sizeof(timestampRedisRequestEnd));
                    string redis_begin_time = resp.headers()["redis_begin_time"];
                    string redis_end_time = resp.headers()["redis_end_time"];

                    resp.extract_json()
                    .then([=] (pplx::task<json::value> task)
                    {
                        json::value regResp;

                        try {
                            regResp = task.get();
                        } catch (std::exception& e) {
                            std::cout << "Redis: EXCEPTION CAUGHT: " << e.what() << std::endl;
                        }

                        string message = regResp["Message"].as_string();
                        string redisToken = regResp["tokenValue"].as_string();

                        //std::cout << message << std::endl;

                        // if(redisToken != discoveryToken || redisToken != token || discoveryToken != token)
                        //     std::cout << "Some issue with tokens" << std::endl;

                        http_response response;

                        HTTP_Response *html_response = handle_request(link);
                        if(html_response->status_code == "200")
                            response.set_status_code(status_codes::OK);
                        else
                        response.set_status_code(status_codes::NotFound);
                        response.headers().add(U("Content-Type"), U("text/html"));
                        response.set_body(utility::conversions::to_string_t(html_response->body));
                        response.headers().add(U("Date"),html_response->time);
                        response.headers().add(U("tokenValue"),token);

                        char timestampEnd[20];
                        getTimestamp(timestampEnd, sizeof(timestampEnd));
                        response.headers().add(U("mainserver_begin_time"),U(timestampStart));
                        response.headers().add(U("mainserver_end_time"),U(timestampEnd));
                        response.headers().add(U("mainserver_discovery_start_time"),U(timestampDiscoveryRequestStart));
                        response.headers().add(U("mainserver_discovery_end_time"),U(timestampDiscoveryRequestEnd));
                        response.headers().add(U("mainserver_redis_start_time"),U(timestampRedisRequestStart));
                        response.headers().add(U("mainserver_redis_end_time"),U(timestampRedisRequestEnd));
                        response.headers().add(U("discovery_begin_time"),U(discovery_begin_time));
                        response.headers().add(U("discovery_end_time"),U(discovery_end_time));
                        response.headers().add(U("redis_begin_time"),U(redis_begin_time));
                        response.headers().add(U("redis_end_time"),U(redis_end_time));

                        request.reply(response);

                    });

                });
            });
    });
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
