#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include "redis_repository.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

const utility::string_t SERVER_URL = U("http://127.0.0.1:5005");

void handle_get(const http_request& request)
{

    utility::string_t path = request.request_uri().path();

    redis_data_repository repository;
    // try{
    //     repository.storeData();
    // } catch (std::exception &e) {
    //     std::cout << "Redis Store: EXCEPTION CAUGHT: " << e.what() << std::endl;
    // }
    web::json::value response;
    try {
        response = repository.getData();
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
