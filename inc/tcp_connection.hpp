#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <iostream>
#include <memory>
#include <mutex>
#include "asio.hpp"

using asio::ip::tcp;

class TcpObject
{
public:
    virtual void onRead(int connId, std::string data) = 0;
    virtual void onClose(int connId) = 0;
    virtual void onStart(int connId) = 0;
};

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    static std::shared_ptr<TcpConnection> create(tcp::socket &&socket, TcpObject &object, int connId = 0);

    void read();
    void send(const std::string &message);
    void close();
    void doWrite();
    void send(const char *data, size_t size);

private:
    TcpConnection(tcp::socket &&socket, TcpObject &object, int connId);
    TcpObject &m_object;
    tcp::socket m_socket;
    asio::streambuf m_readBuffer;
    asio::streambuf m_writeBuffer;
    std::mutex m_writeBufferMutex;
    int m_connectionId;
    bool m_isWritting;

    std::string message_;
};
#endif