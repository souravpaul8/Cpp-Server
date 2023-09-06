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
const utility::string_t SERVICE_DISCOVERY_SERVER_URL = U("http://localhost:5001");

void handle_get(const http_request& request)
{
    http::uri serviceDiscoveryUri = http::uri(U("http://localhost:5001"));
    http::client::http_client serviceDiscovery(http::uri_builder(serviceDiscoveryUri)
                        .append_path(U("/redis"))
                        .to_uri());

    serviceDiscovery.request(methods::GET)
    .then([=](pplx::task<http_response> task)
    {
        http_response resp;
        try {
            resp = task.get();
        }catch(std::exception &e) {
            std::cout << "ServiceDiscovery: EXCEPTION CAUGHT: " << e.what() << std::endl;
        }

        resp.extract_json()
            .then([=] (pplx::task<json::value> task)
            {
                json::value regResp;
                string message;
                try {
                    regResp = task.get();
                } catch (std::exception& e) {
                    std::cout << "ServiceDiscovery: EXCEPTION CAUGHT: " << e.what() << std::endl;
                }

                message = regResp["message"].as_string();

                http::uri redisServerUri = http::uri(U(message));

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
                        std::cout << "Redis: EXCEPTION CAUGHT: " << e.what() << std::endl;
                    }

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
                        //std::cout << message << std::endl;

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
