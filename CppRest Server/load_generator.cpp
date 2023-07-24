#include <iostream>
#include <vector>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <chrono>
#include <thread>

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace std::chrono_literals;

const std::string SERVER_URL = "http://localhost:5000";
const std::vector<std::string> REQUEST_PATHS = { "/", "/apart1/", "/apart2/", "/apart1/flat11/",
                                                 "/apart1/flat12/", "/apart2/flat21/", "/apart3/flat31/", "/apart3/flat32/" };

struct user_info {
    int id;
    float think_time;
    int total_count;
    float total_rtt;
};

void user_function(const std::shared_ptr<http_client>& client, const std::shared_ptr<user_info>& info) {
    while (true) {
        auto start = std::chrono::steady_clock::now();

        // Send GET request to the server
        http_request request(methods::GET);
        request.set_request_uri(REQUEST_PATHS[rand() % REQUEST_PATHS.size()]);

        client->request(request).then([&](http_response response) {
            if (response.status_code() == status_codes::OK) {
                info->total_count++;
            }
            else {
                std::cerr << "Error: Failed to get response" << std::endl;
            }
        }).wait();

        auto end = std::chrono::steady_clock::now();
        auto rtt = std::chrono::duration_cast<std::chrono::duration<float>>(end - start).count();
        info->total_rtt += rtt;

        if (info->total_rtt < info->think_time)
            std::this_thread::sleep_for(std::chrono::duration<float>(info->think_time - info->total_rtt));
        else
            break;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <number of concurrent users> <think time (in s)> <test duration (in s)>" << std::endl;
        return 1;
    }

    int user_count = std::stoi(argv[1]);
    float think_time = std::stof(argv[2]);
    int test_duration = std::stoi(argv[3]);

    std::cout << "Number of Concurrent Users: " << user_count << std::endl;
    std::cout << "Think Time: " << think_time << " s" << std::endl;
    std::cout << "Test Duration: " << test_duration << " s" << std::endl;

    // Create HTTP client
    auto client = std::make_shared<http_client>(SERVER_URL);

    // Create user threads
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<user_info>> user_infos;
    for (int i = 0; i < user_count; ++i) {
        auto info = std::make_shared<user_info>();
        info->id = i;
        info->think_time = think_time;
        info->total_count = 0;
        info->total_rtt = 0;

        user_infos.push_back(info);
        threads.emplace_back(user_function, client, info);
    }

    // Sleep for the test duration
    std::this_thread::sleep_for(std::chrono::seconds(test_duration));

    // Join user threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Calculate and print results
    int total_count = 0;
    float total_rtt = 0;
    for (const auto& info : user_infos) {
        total_count += info->total_count;
        total_rtt += info->total_rtt;
    }

    float avg_rtt = (total_count > 0) ? (total_rtt / total_count) : 0.0f;
    float throughput = total_count / static_cast<float>(test_duration);
    std::cout << "Average Throughput: " << throughput << " requests/s" << std::endl;
    std::cout << "Average RTT: " << avg_rtt << " s" << std::endl;

    return 0;
}
