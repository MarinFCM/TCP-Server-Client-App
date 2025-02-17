#include "tcp_server.hpp"

int main(int argc, char* argv[]){
    if(argc < 2){
        std::cout << "Invalid argument count" << std::endl;
        return -1;
    }
    int x = atoi(argv[1]);
    std::cout << "port number: " << x << std::endl;
    asio::io_context io_context;
    TcpServer server = TcpServer(x, io_context);
    io_context.run();
    return 0;
}