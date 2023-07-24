#include <cpprest/http_listener.h>
#include <cpprest/json.h>

#include "http_server.hh"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

const utility::string_t SERVER_URL = U("http://localhost:8080");

void handle_get(const http_request& request)
{
    //std::cout << "Inside Handle Get" << std::endl;
    http_response response;

    utility::string_t path = request.request_uri().path();
    //std::cout << path << std::endl;
    HTTP_Response *html_response = handle_request(path);
    if(html_response->status_code == "200")
        response.set_status_code(status_codes::OK);
    else
       response.set_status_code(status_codes::NotFound);
    response.headers().add(U("Content-Type"), U("text/html"));
    response.set_body(utility::conversions::to_string_t(html_response->body));
    response.headers().add(U("Date"),html_response->time);
    request.reply(response);
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
