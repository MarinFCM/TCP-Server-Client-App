#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <iostream>
#include "asio.hpp"
#include <map>

using asio::ip::tcp;

class TcpServer {
    public:
    TcpServer(int port, asio::io_context& io_context)
    :   m_io_context(io_context),
        m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)),
        m_serverPort(port),
        m_clientCount(0) {}
    private:
        asio::io_context& m_io_context;
        tcp::acceptor m_acceptor;

        int m_serverPort;
        int m_clientCount;
        std::unordered_map<std::string, std::string> m_clientTopics;
};

#endif