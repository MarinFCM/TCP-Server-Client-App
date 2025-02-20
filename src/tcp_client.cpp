#include "tcp_client.hpp"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mutex>
#include <queue>
#include <bits/this_thread_sleep.h>

std::mutex outputMutex;
std::mutex inputMutex;
std::atomic<bool> exit_flag(false);
std::queue<std::string> inputQueue;

TcpClient::TcpClient(boost::asio::io_context &ioContext) : m_ioContext(ioContext), m_connection{}, m_isConnected{false} {}

void TcpClient::connect(const int& port, const std::string& name) {
    if (!m_isConnected) {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_ioContext);
        boost::system::error_code ec;
        socket->connect(tcp::endpoint{boost::asio::ip::address::from_string("127.0.0.1"), port}, ec); 
        if (ec) {
            std::cerr << "TcpClient::connect() error: " + ec.message() + ".\n";
            onClose(0);
            return;
        }
        m_connection = TcpConnection::create(std::move(*socket), *this);
        m_isConnected = true;
        m_connection->read();
        onStart(0);
        m_serverPort = port;
        m_clientName = name;
        std::string connectString = "CONNECT;" + name;
        m_connection->send(connectString.c_str(), connectString.size());
        std::cout << "Connected to port " << m_serverPort << " as " << m_clientName << std::endl;
    } else {
        std::cout << "Already connected to " << m_serverPort << std::endl;
    }
}

void TcpClient::disconnect() {
    if (m_isConnected) {
        std::string connectString = "DISCONNECT;";
        m_connection->send(connectString.c_str(), connectString.size());
    } else {
        std::cout << "Not connected to any server." << std::endl;
    }
}

void TcpClient::publish(const std::string& topic, const std::string& data) {
    if (m_isConnected) {
        std::cout << "Published to topic: " << topic << " with data: " << data << std::endl;
        std::string connectString = "PUBLISH;" + topic + ";" + data;
        m_connection->send(connectString.c_str(), connectString.size());
    } else {
        std::cout << "You must be connected to publish." << std::endl;
    }
}

void TcpClient::subscribe(const std::string& topic) {
    if (m_isConnected) {
        auto it = std::find(m_topics.begin(), m_topics.end(), topic);
        if(it == m_topics.end()){
            std::string connectString = "SUBSCRIBE;" + topic;
            m_connection->send(connectString.c_str(), connectString.size());
            m_topics.push_back(topic);
            std::cout << "Subscribed to topic: " << topic << std::endl;
        }
        else{
            std::cout << "Already subscribed to topic: " << topic << std::endl;
        }
    } else {
        std::cout << "You must be connected to subscribe." << std::endl;
    }
}

void TcpClient::unsubscribe(const std::string& topic) {
    if (m_isConnected) {
        auto it = std::find(m_topics.begin(), m_topics.end(), topic);
        if (it != m_topics.end()) {
            std::string connectString = "UNSUBSCRIBE;" + topic;
            m_connection->send(connectString.c_str(), connectString.size());
            m_topics.erase(it);
            std::cout << "Unsubscribed from topic: " << topic << std::endl;
        } else {
            std::cout << "Not subscribed to topic: " << topic << std::endl;
        }
    } else {
        std::cout << "You must be connected to unsubscribe." << std::endl;
    }
}

void TcpClient::onRead(int connId, std::string payload) {
    std::istringstream stream(payload);
    std::string topic, data;
    if (!std::getline(stream, topic, ';')) {
        return;
    }
    std::getline(stream, data);

    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << "[Message] Topic: " << topic << " Data: " << data << std::endl;
}

void TcpClient::handleCommand(const std::string& input, int connid) {
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
        std::cout << "Invalid command: " << input << std::endl;
    }
}

void TcpClient::handleConnect(std::istringstream& stream, int connid) {
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

    connect(port, name);
}

void TcpClient::handleDisconnect(int connid) {
    disconnect();
}

void TcpClient::handlePublish(std::istringstream& stream, int connid) {
    std::string topic, data;
    stream >> topic;
    std::getline(stream, data);
    data.erase(0, 1);
    
    if (topic.empty() || data.empty()) {
        std::cout << "Error: PUBLISH command requires <topic> and <data> parameters.\n";
    } else {
        publish(topic, data);
    }
}

void TcpClient::handleSubscribe(std::istringstream& stream, int connid) {
    std::string topic;
    stream >> topic;
    if (topic.empty()) {
        std::cout << "Error: SUBSCRIBE command requires <topic> parameter.\n";
    } else {
        subscribe(topic);
    }
}

void TcpClient::handleUnsubscribe(std::istringstream& stream, int connid) {
    std::string topic;
    stream >> topic;
    if (topic.empty()) {
        std::cout << "Error: UNSUBSCRIBE command requires <topic> parameter.\n";
    } else {
        unsubscribe(topic);
    }
}

void TcpClient::onClose(int connId){
    m_isConnected = false;
    m_topics.clear();
    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << "Connection to server closed" << std::endl;
}

void TcpClient::onStart(int connId){
    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << "Connection to server started" << std::endl;
}

void signal_handler(int s){
    std::cout << std::endl << "Caught SIGINT signal" << std::endl;
    exit(1); 
}

std::string get_input(){
    std::string cmd;
    std::cin >> cmd;
    return cmd;
}

void read_input() {
    std::string input;
    while (!exit_flag) {
        std::getline(std::cin, input);
        if (!input.empty()) {
            std::lock_guard<std::mutex> lock(inputMutex);
            inputQueue.push(input);
            if (input == "exit") {
                exit_flag = true;
            }
        }
    }
}

int main() {
    boost::asio::io_context context;

    TcpClient client{context};

    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work = boost::asio::make_work_guard(context);
    std::thread thread{[&context]() { context.run(); }};

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    std::thread input_thread(read_input);

    while (!exit_flag) {
        {
            std::lock_guard<std::mutex> lock(inputMutex);
            if (!inputQueue.empty()) {
                std::string user_input = inputQueue.front();
                inputQueue.pop();
                client.handleCommand(user_input);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    work.reset();
    input_thread.join();
    thread.join();
    return 0;
}