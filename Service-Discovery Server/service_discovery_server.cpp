#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <boost/algorithm/string.hpp>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

const utility::string_t SERVER_URL = U("http://localhost:5001");

void handle_get(const http_request& request)
{

    utility::string_t path = request.request_uri().path();
    std::vector<utility::string_t> segments;
    boost::split(segments, path, boost::is_any_of("/"), boost::token_compress_on);
    //std::string pathString = utility::conversions::to_utf8string(path);
    //std::cout << segments[1] << std::endl;
    std::string message;
    //std::cout << message.length() << std::endl;

    if (segments.size() > 1) {
        if (segments[1] == "ausf") {
            message = "http://localhost:5001";
        } else if (segments[1] == "redis") {
            message = "http://localhost:5005";
        } else {
            std::cout << "The path does not match any segment" << std::endl;
        }
    } else {
        std::cout << "The path does not contain enough segments." << std::endl;
    }


    json::value response;
    response[U("message")] = json::value::string(U(message));

    utility::string_t responseString = response.serialize();

    http_response httpResponse;
    if(message.length() == 0)
        http_response httpResponse(status_codes::NotFound);
    else
        http_response httpResponse(status_codes::OK);

    httpResponse.headers().set_content_type(U("application/json"));
    httpResponse.set_body(responseString);
    request.reply(httpResponse);
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