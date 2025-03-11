#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// Server function: Listens for a client message and responds
void run_server() {
    try {
        boost::asio::io_context io_context;

        // Listen on port 12345
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));
        std::cout << "Server listening on port 12345...\n";

        // Accept a connection
        tcp::socket socket(io_context);
        acceptor.accept(socket);
        std::cout << "Client connected!\n";

        // Read the client's message
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, "\n");
        std::istream input(&buffer);
        std::string client_message;
        std::getline(input, client_message);
        std::cout << "Received from client: " << client_message << "\n";

        // Send acknowledgment
        std::string response = "ACK " + client_message + "\n";
        boost::asio::write(socket, boost::asio::buffer(response));
        std::cout << "Sent to client: " << response;

        // Close the socket
        socket.close();
    }
    catch (const std::exception& e) {
        std::cerr << "Server exception: " << e.what() << "\n";
    }
}

// Client function: Connects to server and sends a game-like message
void run_client() {
    try {
        boost::asio::io_context io_context;

        // Resolve localhost:12345
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "12345");

        // Connect to the server
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);
        std::cout << "Connected to server!\n";

        // Send a game-like message (e.g., player movement)
        std::string message = "MOVE 10 20\n";
        boost::asio::write(socket, boost::asio::buffer(message));
        std::cout << "Sent to server: " << message;

        // Read the server's response
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, "\n");
        std::istream input(&buffer);
        std::string server_response;
        std::getline(input, server_response);
        std::cout << "Received from server: " << server_response << "\n";

        // Close the socket
        socket.close();
    }
    catch (const std::exception& e) {
        std::cerr << "Client exception: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <server|client>\n";
        return 1;
    }

    std::string mode = argv[1];
    if (mode == "server") {
        run_server();
    }
    else if (mode == "client") {
        run_client();
    }
    else {
        std::cerr << "Invalid mode. Use 'server' or 'client'.\n";
        return 1;
    }

    return 0;
}