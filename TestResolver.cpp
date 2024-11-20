#include "DnsResolver.h"

void runServer() {
    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, {tcp::v4(), 8080});

    DnsResolver resolver("postgres", "daniilsavin", "/tmp", 5432);

    while (true) {
        tcp::socket socket(io_context);
        acceptor.accept(socket);

        boost::beast::flat_buffer buffer;
        http::request<http::string_body> request;
        boost::beast::http::read(socket, buffer, request);

        resolver.handleRequest(request, socket);

        socket.shutdown(tcp::socket::shutdown_send);
    }
}

int main() {
    try {
        runServer();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
