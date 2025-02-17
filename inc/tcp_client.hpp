#include <iostream>
#include "asio.hpp"
#include <vector>

class TcpClient {
    public:
        TcpClient() : m_isConnected(false) {}

        void connect(const int& port, const std::string& name);
        void disconnect();
        void publish(const std::string& topic, const std::string& data);
        void subscribe(const std::string& topic);
        void unsubscribe(const std::string& topic);
        bool isConnectedStatus() const {
            return m_isConnected;
        };

    private:
        bool m_isConnected;
        int m_serverPort;
        std::string m_clientName;
        std::vector<std::string> m_topics;
};

class ClientCommandHandler {
    public:
    ClientCommandHandler(TcpClient& client) : client(client) {}
    
        void handleCommand(const std::string& input) {
            std::istringstream stream(input);
            std::string command;
            stream >> command;
    
            if (command == "CONNECT") {
                handleConnect(stream);
            }
            else if (command == "DISCONNECT") {
                handleDisconnect();
            }
            else if (command == "PUBLISH") {
                handlePublish(stream);
            }
            else if (command == "SUBSCRIBE") {
                handleSubscribe(stream);
            }
            else if (command == "UNSUBSCRIBE") {
                handleUnsubscribe(stream);
            }
            else {
                std::cout << "Invalid command: " << command << std::endl;
            }
        }
    
    private:
        void handleConnect(std::istringstream& stream) {
            std::string portStr, name;
            stream >> portStr >> name;

            if (portStr.empty() || name.empty()) {
                std::cout << "Error: CONNECT command requires <port> (int) and <name> parameters.\n";
                return;
            }

            int port = strtol(portStr.c_str(), nullptr, 10);

            if (port < 1 || port > 65535) {
                std::cout << "Error: Port must be an integer between 1 and 65535.\n";
                return;
            }

            client.connect(port, name);
        }
    
        void handleDisconnect() {
            client.disconnect();
        }
    
        void handlePublish(std::istringstream& stream) {
            std::string topic, data;
            stream >> topic;
            std::getline(stream, data);
            if (topic.empty() || data.empty()) {
                std::cout << "Error: PUBLISH command requires <topic> and <data> parameters.\n";
            } else {
                client.publish(topic, data);
            }
        }
    
        void handleSubscribe(std::istringstream& stream) {
            std::string topic;
            stream >> topic;
            if (topic.empty()) {
                std::cout << "Error: SUBSCRIBE command requires <topic> parameter.\n";
            } else {
                client.subscribe(topic);
            }
        }
    
        void handleUnsubscribe(std::istringstream& stream) {
            std::string topic;
            stream >> topic;
            if (topic.empty()) {
                std::cout << "Error: UNSUBSCRIBE command requires <topic> parameter.\n";
            } else {
                client.unsubscribe(topic);
            }
        }
    
        TcpClient& client;
};