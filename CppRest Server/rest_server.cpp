#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include "cpprest/uri.h"
#include <chrono>

#include "http_server.hh"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

const int timeoutSeconds = 5;

const utility::string_t SERVER_URL = U("http://localhost:80");

void handle_get(const http_request& request)
{
    http::uri redisServerUri = http::uri(U("http://localhost:5000"));
    http::client::http_client redis(http::uri_builder(redisServerUri)
                        .append_path(U("/"))
                        .to_uri());

    redis.request(methods::GET)
    .then([=](pplx::task<http_response> task)
    {
        http_response resp;
        try {
            resp = task.get();
        }catch(std::exception &e) {
            std::cout << "EXCEPTION CAUGHT: " << e.what() << std::endl;
        }

        if(resp.status_code() == status_codes::OK){
            resp.content_ready().then([=](web::http::http_response response) {
                std::string htmlContent = response.to_string();
                //std::cout << "HTML Response: " << htmlContent << std::endl;
            });
        } else {
            std::cout << "HTTP Response Error: " << resp.status_code() << std::endl;
        }
        http_response response;

        utility::string_t path = request.request_uri().path();

        HTTP_Response *html_response = handle_request(path);
        if(html_response->status_code == "200")
            response.set_status_code(status_codes::OK);
        else
        response.set_status_code(status_codes::NotFound);
        response.headers().add(U("Content-Type"), U("text/html"));
        response.set_body(utility::conversions::to_string_t(html_response->body));
        response.headers().add(U("Date"),html_response->time);
        request.reply(response);
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
