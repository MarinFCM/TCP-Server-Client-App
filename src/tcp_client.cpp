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

TcpClient::TcpClient(asio::io_context &ioContext) : m_ioContext(ioContext), m_isConnected(false), m_connection{} {}

void TcpClient::connect(const int& port, const std::string& name) {
    if (!m_isConnected) {
        auto socket = std::make_shared<asio::ip::tcp::socket>(m_ioContext);
        asio::error_code ec;
        socket->connect(tcp::endpoint{asio::ip::address::from_string("127.0.0.1"), port}, ec); 
        if (ec) {
            std::cerr << "TcpClient::connect() error: " + ec.message() + ".\n";
            onClose(0);
            return;
        }
        m_connection = TcpConnection::create(std::move(*socket), *this);
        m_connection->read();
        onStart(0);
        m_serverPort = port;
        m_clientName = name;
        m_isConnected = true;
        std::string connectString = "CONNECT;" + name;
        m_connection->send(connectString.c_str(), connectString.size());
        std::cout << "Connected to port " << m_serverPort << " as " << m_clientName << std::endl;
    } else {
        std::cout << "Already connected to " << m_serverPort << std::endl;
    }
}

void TcpClient::disconnect() {
    if (m_isConnected) {
        m_connection->close();
        m_isConnected = false;
        std::cout << "Disconnected from port " << m_serverPort << std::endl;
    } else {
        std::cout << "Not connected to any server." << std::endl;

    }
}

void TcpClient::publish(const std::string& topic, const std::string& data) {
    if (m_connection) {
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

void TcpClient::onClose(int connId){
    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << "Connection closed to " << connId << std::endl;
}

void TcpClient::onStart(int connId){
    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << "Connection started to " << connId << std::endl;
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
    asio::io_context context;

    TcpClient client{context};

    asio::executor_work_guard<asio::io_context::executor_type> work = asio::make_work_guard(context);
    std::thread thread{[&context]() { context.run(); }};

    ClientCommandHandler handler(client);

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
                if(user_input == "DISCONNECT") work.reset();
                handler.handleCommand(user_input);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    input_thread.join();
    thread.join();
    return 0;
}