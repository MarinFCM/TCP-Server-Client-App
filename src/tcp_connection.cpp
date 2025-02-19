#include "tcp_connection.hpp"

TcpConnection::TcpConnection(tcp::socket &&socket, TcpObject &object, int connId) : m_socket(std::move(socket)), m_object(object), m_connectionId(connId), m_writeBuffer{},
m_writeBufferMutex{}, m_isWritting{false} {}

void TcpConnection::read(){
    auto buffers = m_readBuffer.prepare(512);
    auto self = shared_from_this();
    m_socket.async_read_some(buffers, [this, self](const auto &error,
                                                    auto bytesTransferred) {
        if (error) {
            if(error.value() != 2){
                std::cerr << "TcpConnection::doRead() error: " << error.value() << " "<< error.message() <<".\n";
            }
            return close();
        }
        m_readBuffer.commit(bytesTransferred);
        //std::cout << "Received message: " << std::string(static_cast<const char *>(m_readBuffer.data().data()), bytesTransferred) << std::endl;
        m_object.onRead(m_connectionId,
            std::string(static_cast<const char *>(m_readBuffer.data().data()), bytesTransferred));
        m_readBuffer.consume(bytesTransferred);
        read();
    });
}

void TcpConnection::send(const char *data, size_t size) {
    if (m_socket.is_open()) {
        std::ostream bufferStream{&m_writeBuffer};
        bufferStream.write(data, size);
        if (!m_isWritting) {
            doWrite();
        }
    }
    else{
        std::cerr << "Socket is closed.\n";
    }
}

void TcpConnection::doWrite() {
    m_isWritting = true;
    auto self = shared_from_this();
    asio::error_code error;
    auto bytesTransferred = m_socket.write_some(m_writeBuffer.data(), error);
      if (error) {
        std::cerr << "TcpConnection::doWrite() error: " + error.message() + ".\n";
        close();
        return;
      }
      m_writeBuffer.consume(bytesTransferred);
      if (m_writeBuffer.size() == 0) {
        m_isWritting = false;
        return;
      }
      doWrite();
  }

void TcpConnection::close(){
    try {
        m_socket.cancel();
        m_socket.close();
    } catch (const std::exception &e) {
        std::cerr << "TcpConnection::close() exception: " +
                        static_cast<std::string>(e.what()) + ".\n";
        return;
    }
    m_object.onClose(m_connectionId);
}

std::shared_ptr<TcpConnection> TcpConnection::create(tcp::socket &&socket, TcpObject &object, int connId){
    return std::shared_ptr<TcpConnection>(
        new TcpConnection{std::move(socket), object, connId});
}
