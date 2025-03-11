#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;

        // Resolve the endpoint (e.g., connect to example.com on port 80)
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("example.com", "http");

        // Create a socket and connect
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        // Send a simple HTTP GET request
        std::string request = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
        boost::asio::write(socket, boost::asio::buffer(request));

        // Read the response
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        // Output the response
        std::istream response_stream(&response);
        std::string http_version;
        unsigned int status_code;
        std::string status_message;

        response_stream >> http_version >> status_code;
        std::getline(response_stream, status_message);
        std::cout << "Response: " << http_version << " " << status_code << status_message << "\n";

        // Close the socket
        socket.close();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}