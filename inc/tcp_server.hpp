#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <iostream>
#include "asio.hpp"
#include "tcp_connection.hpp"
#include "command_handler.hpp"
#include <map>

using asio::ip::tcp;

class TcpServer : TcpObject{
    public:
        TcpServer(int port, asio::io_context& io_context);
        void onRead(int connId, std::string data) override;
        void onClose(int connId) override;
        void onStart(int connId) override;

        void start();
        void handleCommand(const std::string& input, int connId);
    private:
        void accept();

        void handleConnect(std::istringstream& stream, int connId);
        void handleDisconnect(int connId);
        void handlePublish(std::istringstream& stream, int connId);
        void handleSubscribe(std::istringstream& stream, int connId);
        void handleUnsubscribe(std::istringstream& stream, int connId);

        asio::io_context& m_ioContext;
        tcp::acceptor m_acceptor;

        int m_serverPort;
        int m_clientCount;
        std::unordered_map<int, std::vector<std::string>> m_clientTopics;
        std::unordered_map<int, std::shared_ptr<TcpConnection>> m_clientConnections;
        std::unordered_map<int, std::string> m_clientNames;
};

#endif