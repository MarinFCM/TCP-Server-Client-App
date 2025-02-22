#include "tcp_server.hpp"
#include <algorithm>

TcpServer::TcpServer(int port, boost::asio::io_context& io_context) : 
    m_ioContext(io_context),
    m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)),
    m_serverPort(port),
    m_clientCount(0) {}

void TcpServer::start(){
    try {
        std::cout << "Starting server on port " << m_serverPort << std::endl;
        m_acceptor.listen(Constants::max_clients);
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

int TcpServer::getClientCount() const{
    return m_clientNames.size();
}

std::string TcpServer::getClientName(int connId) const{
    if(m_clientNames.find(connId) != m_clientNames.end()){
        return m_clientNames.at(connId);
    }
    return "";
}

std::vector<std::string> TcpServer::getClientTopics(int connId) const{
    if(m_clientTopics.find(connId) != m_clientTopics.end()){
        return m_clientTopics.at(connId);
    }
    return std::vector<std::string>();
}

void TcpServer::handleCommand(const std::string& input, int connId){
    std::istringstream stream(input);
    std::string command;
    if (!std::getline(stream, command, Constants::delimiter.c_str()[0])) {
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
    m_clientConnections[connId]->close();
}

void TcpServer::handlePublish(std::istringstream& stream, int connId){
    (void)connId;
    std::string topic, data;
    if (!std::getline(stream, topic, Constants::delimiter.c_str()[0])) {
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
            std::string sendData(topic + Constants::delimiter + data);
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
        }       
    }
}


void TcpServer::onRead(int connId, std::string data) {
    handleCommand(data, connId);
}

void TcpServer::onClose(int connId){
    if(m_clientNames.find(connId) != m_clientNames.end()){
        std::cout << "Connection closed to client(id="<<connId<<") " << m_clientNames[connId] << std::endl;
        m_clientTopics.erase(connId);
        m_clientNames.erase(connId);
        m_clientConnections.erase(connId);
    }
}

void TcpServer::onStart(int connId){
    std::cout << "New client(id="<<connId<<") connected" << std::endl;
}

void signal_handler(int s){
    (void)s;
    std::cout << std::endl << "Caught SIGINT signal" << std::endl;
    exit(1); 
}

int main(int argc, char* argv[]){
    if(argc != 2){
        std::cout << "Program only takes one input: <server_port>" << std::endl;
        return -1;
    }

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    int x = atoi(argv[1]);
    boost::asio::io_context context;

    TcpServer server{x, context};
    server.start();
    context.run();

    return 0;
}