#include <cpprest/http_listener.h>
#include <cpprest/json.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

const utility::string_t SERVER_URL = U("http://localhost:5000");

void handle_get(const http_request& request)
{

    utility::string_t path = request.request_uri().path();

    std::string response = "<!DOCTYPE html> <html lang=\"en\">\
                        <html>\
                        <head>\
                            <title>Hello Sourav</title>\
                        </head>\
                        <body>\
                            <p>Hello Sourav</p>\
                        </body>\
                        </html>";

    http_response httpResponse(status_codes::OK);
    httpResponse.headers().set_content_type(U("text/html"));
    httpResponse.set_body(utility::conversions::to_string_t(response));
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
