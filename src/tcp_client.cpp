#include "tcp_client.hpp"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void TcpClient::connect(const int& port, const std::string& name) {
    if (!m_isConnected) {
        m_serverPort = port;
        m_clientName = name;
        m_isConnected = true;
        std::cout << "Connected to port " << m_serverPort << " as " << m_clientName << std::endl;
    } else {
        std::cout << "Already connected to " << m_serverPort << std::endl;
    }
}

void TcpClient::disconnect() {
    if (m_isConnected) {
        m_isConnected = false;
        std::cout << "Disconnected from port " << m_serverPort << std::endl;
    } else {
        std::cout << "Not connected to any server." << std::endl;
    }
}

void TcpClient::publish(const std::string& topic, const std::string& data) {
    if (m_isConnected) {
        std::cout << "Published to topic: " << topic << " with data: " << data << std::endl;
    } else {
        std::cout << "You must be connected to publish." << std::endl;
    }
}

void TcpClient::subscribe(const std::string& topic) {
    if (m_isConnected) {
        auto it = std::find(m_topics.begin(), m_topics.end(), topic);
        if(it == m_topics.end()){
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
            m_topics.erase(it);
            std::cout << "Unsubscribed from topic: " << topic << std::endl;
        } else {
            std::cout << "Not subscribed to topic: " << topic << std::endl;
        }
    } else {
        std::cout << "You must be connected to unsubscribe." << std::endl;
    }
}

void signal_handler(int s){
    std::cout << std::endl << "Caught SIGINT signal " << std::endl;
    exit(1); 
}

int main() {
    TcpClient client;
    ClientCommandHandler handler(client);
    std::string input;    

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    while (true) {
        std::cout << "Enter a command: ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        handler.handleCommand(input);
    }

    return 0;
}