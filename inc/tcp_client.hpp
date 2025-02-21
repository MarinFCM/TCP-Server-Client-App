#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <iostream>
#include "tcp_connection.hpp"
#include "command_handler.hpp"
#include <vector>

class TcpClient : TcpObject, CommandHandler {
    public:
        void onRead(int connId, std::string payload) override;
        void onClose(int connId) override;
        void onStart(int connId) override;
        bool isConnected() const;
        void handleCommand(const std::string& input, int connId = 0);

        TcpClient(boost::asio::io_context &ioContext);
    private:
        void handleConnect(std::istringstream& stream, int connId = 0);
        void handleDisconnect(int connId = 0);
        void handlePublish(std::istringstream& stream, int connId = 0);
        void handleSubscribe(std::istringstream& stream, int connId = 0);
        void handleUnsubscribe(std::istringstream& stream, int connId = 0);

        void connect(const int& port, const std::string& name);
        void disconnect();
        void publish(const std::string& topic, const std::string& data);
        void subscribe(const std::string& topic);
        void unsubscribe(const std::string& topic);

        int m_serverPort;
        std::string m_clientName;
        std::vector<std::string> m_topics;

        bool m_isConnected;
        std::shared_ptr<TcpConnection> m_connection;
        boost::asio::io_context &m_ioContext;
};

#endif