#include "tcp_server.hpp"
#include <algorithm>

TcpServer::TcpServer(int port, asio::io_context& io_context) : 
    m_ioContext(io_context),
    m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)),
    m_serverPort(port),
    m_clientCount(0) {}

void TcpServer::start(){
    try {
        std::cout << "Starting server on port " << m_serverPort << std::endl;
        m_acceptor.listen(5);
    } catch (const std::exception &e) {
        std::cerr << "TcpServer::start() exception: " +
                         static_cast<std::string>(e.what()) + ".\n";
        return;
    }
      accept();
}

void TcpServer::accept(){
    m_acceptor.async_accept([this](const auto &error, auto socket) {
        if (error) {
          std::cerr << "TcpServer::doAccept() error: " + error.message() + ".\n";
          return;
        } else {
          auto connection{
              TcpConnection::create(std::move(socket), *this, m_clientCount)};
          connection->read();
          m_clientConnections.insert({m_clientCount, std::move(connection)});
          onStart(m_clientCount);
          m_clientCount++;
        }
        accept();
    });
}

void TcpServer::handleCommand(const std::string& input, int connId){
    std::istringstream stream(input);
    std::string command;
    if (!std::getline(stream, command, ';')) {
        return;
    }

    if (command == "CONNECT") {
        handleConnect(stream, connId);
    }
    else if (command == "DISCONNECT") {
        handleDisconnect(connId);
    }
    else if (command == "PUBLISH") {
        handlePublish(stream, connId);
    }
    else if (command == "SUBSCRIBE") {
        handleSubscribe(stream, connId);
    }
    else if (command == "UNSUBSCRIBE") {
        handleUnsubscribe(stream, connId);
    }
    else {
        std::cout << "Invalid command: " << command << std::endl;
    }
}

void TcpServer::handleConnect(std::istringstream& stream, int connId){
    std::string name;
    stream >> name;

    if (name.empty()) {
        std::cout << "Error: Invalid format CONNECT received.\n";
        return;
    }
    m_clientNames[connId] = name;
    m_clientTopics[connId] = std::vector<std::string>();
    std::cout << "Client (id="<<connId<<") name: " << name << std::endl;
}

void TcpServer::handleDisconnect(int connId){
    m_clientTopics.erase(connId);
    m_clientNames.erase(connId);
    m_clientConnections[connId]->close();
    m_clientConnections.erase(connId);
}

void TcpServer::handlePublish(std::istringstream& stream, int connId){
    std::string topic, data;
    if (!std::getline(stream, topic, ';')) {
        return;
    }

    std::getline(stream, data);
    
    if (topic.empty() || data.empty()) {
        std::cout << "Error: Invalid format PUBLISH received.\n";
    } else {
        std::vector<int> topicSubscribers;
        for(auto it : m_clientTopics){
            if(std::find(it.second.begin(), it.second.end(), topic) != it.second.end()){
                topicSubscribers.push_back(it.first);
            }
        }
        for(auto it : topicSubscribers){
            //std::cout << "Sending to subscriber(id="<<it<<").\n";
            std::string sendData(topic+";"+data);
            m_clientConnections[it]->send(sendData.c_str(), sendData.size());
        }
    }
}
void TcpServer::handleSubscribe(std::istringstream& stream, int connId){
    std::string topic;
    stream >> topic;
    if (topic.empty()) {
        std::cout << "Error: Invalid format SUBSCRIBE received.\n";
    } else {
        m_clientTopics[connId].push_back(topic);
        //std::cout << "Client (id="<<connId<<") subscribed to topic: " << topic << std::endl;
    }
}
void TcpServer::handleUnsubscribe(std::istringstream& stream, int connId){
    std::string topic;
    stream >> topic;
    if (topic.empty()) {
        std::cout << "Error: Invalid format UNSUBSCRIBE received.\n";
    } else {
        auto &vecRef = m_clientTopics[connId];
        auto it = std::find(vecRef.begin(), vecRef.end(), topic);
        if(it != vecRef.end()){
            vecRef.erase(it);
            //std::cout << "Client (id="<<connId<<") unsubscribed from topic: " << topic << std::endl;
        }
        else{
            //std::cout << "Client (id="<<connId<<") was not subscribed to topic: " << topic << std::endl;
        }        
    }
}


void TcpServer::onRead(int connId, std::string data) {
    std::cout << "Received from " << connId << ": " << data << std::endl;
    handleCommand(data, connId);
}

void TcpServer::onClose(int connId){
    std::cout << "Connection closed to " << connId << std::endl;
    handleDisconnect(connId);
}

void TcpServer::onStart(int connId){
    std::cout << "Connection started to " << connId << std::endl;
}

int main(int argc, char* argv[]){
    if(argc < 2){
        std::cout << "Invalid argument count" << std::endl;
        return -1;
    }
    int x = atoi(argv[1]);
    std::cout << "port number: " << x << std::endl;
    asio::io_context context;
    //std::thread thread{[&context]() { context.run(); }};

    TcpServer server{x, context};
    server.start();
    context.run();

    //thread.join();
    return 0;
}