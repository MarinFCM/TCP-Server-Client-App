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

inline void printMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << message << std::endl;
}

TcpClient::TcpClient(boost::asio::io_context &ioContext) : 
        m_ioContext(ioContext), 
        m_connection{},
        m_isConnected{false} {}

void TcpClient::connect(const int& port, const std::string& name) {
    if (!m_isConnected) {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_ioContext);
        boost::system::error_code ec;
        socket->connect(tcp::endpoint{boost::asio::ip::address::from_string("127.0.0.1"), static_cast<short unsigned int>(port)}, ec); 
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
        std::string connectString = "CONNECT" + Constants::delimiter + name;
        m_connection->send(connectString.c_str(), connectString.size());
    } else {
        printMessage("Already connected to " + std::to_string(m_serverPort));
    }
}

void TcpClient::disconnect() {
    if (m_isConnected) {
        std::string connectString = "DISCONNECT" + Constants::delimiter;
        m_connection->send(connectString.c_str(), connectString.size());
    } else {
        printMessage("Not connected to any server.");
    }
}

void TcpClient::publish(const std::string& topic, const std::string& data) {
    if (m_isConnected) {
        std::string connectString = "PUBLISH" + Constants::delimiter + topic + Constants::delimiter + data;
        if(m_connection->send(connectString.c_str(), connectString.size())){
            std::cout << "Published to topic: " << topic << " Data: " << data << std::endl;
        }
    } else {
        printMessage("You must be connected to publish.");
    }
}

void TcpClient::subscribe(const std::string& topic) {
    if (m_isConnected) {
        auto it = std::find(m_topics.begin(), m_topics.end(), topic);
        if(it == m_topics.end()){
            std::string connectString = "SUBSCRIBE" + Constants::delimiter + topic;
            if(m_connection->send(connectString.c_str(), connectString.size())){
                printMessage("Subscribed to topic: " + topic);
                m_topics.push_back(topic);
            }
        }
        else{
            printMessage("Already subscribed to topic: " + topic);
        }
    } else {
        printMessage("You must be connected to subscribe.");
    }
}

void TcpClient::unsubscribe(const std::string& topic) {
    if (m_isConnected) {
        auto it = std::find(m_topics.begin(), m_topics.end(), topic);
        if (it != m_topics.end()) {
            std::string connectString = "UNSUBSCRIBE" + Constants::delimiter + topic;
            if(m_connection->send(connectString.c_str(), connectString.size())){
                m_topics.erase(it);
                printMessage("Unsubscribed from topic: " + topic);
            }
        } else {
            printMessage("Not subscribed to topic: " + topic);
        }
    } else {
        printMessage("You must be connected to unsubscribe.");
    }
}

void TcpClient::onRead(int connId, std::string payload) {
    (void)connId;
    std::istringstream stream(payload);
    std::string topic, data;
    if (!std::getline(stream, topic, ';')) {
        return;
    }
    std::getline(stream, data);

    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << "[Message] Topic: " << topic << " Data: " << data << std::endl;
}

bool TcpClient::isConnected() const {
    return m_isConnected;
}

void TcpClient::handleCommand(const std::string& input, int connId) {
    (void)connId;
    
    if(input.length() >= Constants::max_length){
        printMessage("Command length exceeds maximum message length of " + std::to_string(Constants::max_length));
        return;
    }
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
        printMessage("Invalid command: " + input);
    }
}

void TcpClient::handleConnect(std::istringstream& stream, int connId) {
    (void)connId;
    std::string portStr, name;
    stream >> portStr >> name;

    if (portStr.empty() || name.empty()) {
        printMessage("Error: CONNECT command requires <port> (int) and <name> parameters.");
        return;
    }

    int port = strtol(portStr.c_str(), nullptr, 10);

    if (port < 1 || port > 65535) {
        printMessage("Error: Port must be an integer between 1 and 65535.");
        return;
    }

    connect(port, name);
}

void TcpClient::handleDisconnect(int connId) {
    (void)connId;
    disconnect();
}

void TcpClient::handlePublish(std::istringstream& stream, int connId) {
    (void)connId;
    std::string topic, data;
    stream >> topic;
    if(topic.find(Constants::delimiter) != std::string::npos){
        printMessage("Topic contains delimiter character " + Constants::delimiter + " which could lead to unwanted behaviour.");
        return;
    }
    std::getline(stream, data);
    data.erase(0, 1);
    
    if (topic.empty() || data.empty()) {
        printMessage("Error: PUBLISH command requires <topic> and <data> parameters.");
    } else {
        publish(topic, data);
    }
}

void TcpClient::handleSubscribe(std::istringstream& stream, int connId) {
    (void)connId;
    std::string topic;
    stream >> topic;
    if (topic.empty()) {
        printMessage("Error: SUBSCRIBE command requires <topic> parameter.");
    } else {
        if(topic.find(Constants::delimiter) != std::string::npos){
            printMessage("Topic contains delimiter character " + Constants::delimiter + " which could lead to unwanted behaviour.");
            return;
        }
        subscribe(topic);
    }
}

void TcpClient::handleUnsubscribe(std::istringstream& stream, int connId) {
    (void)connId;
    std::string topic;
    stream >> topic;
    if (topic.empty()) {
        printMessage("Error: UNSUBSCRIBE command requires <topic> parameter.");
    } else {
        unsubscribe(topic);
    }
}

void TcpClient::onClose(int connId){
    (void)connId;
    m_isConnected = false;
    m_topics.clear();
    printMessage("Connection to server closed");
}

void TcpClient::onStart(int connId){
    (void)connId;
    printMessage("Connection to server started");
}

void signal_handler(int s){
    (void)s;
    printMessage("Caught SIGINT signal");
    exit_flag = true;
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

        // Prevent 100% CPU usage and allow other threads to run without buffer interrupts
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    work.reset();
    input_thread.join();
    thread.join();
    return 0;
}