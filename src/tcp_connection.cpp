#include "tcp_connection.hpp"

TcpConnection::TcpConnection(tcp::socket &&socket, TcpObject &object, int connId) : m_socket(std::move(socket)), m_object(object), m_readBuffer{}, m_writeBuffer{},
m_writeBufferMutex{}, m_connectionId(connId), m_isWritting{false} {}

void TcpConnection::read(){
    auto buffers = m_readBuffer.prepare(Constants::max_length);
    auto self = shared_from_this();
    m_socket.async_read_some(buffers, [this, self](const auto &error,
                                                    auto bytesTransferred) {
        if (error) {
            return close();
        }
        m_readBuffer.commit(bytesTransferred);
        m_object.onRead(m_connectionId,
            std::string(static_cast<const char *>(m_readBuffer.data().data()), bytesTransferred));
        m_readBuffer.consume(bytesTransferred);
        read();
    });
}

bool TcpConnection::send(const char *data, size_t size) {
    if (m_socket.is_open()) {
        std::ostream bufferStream{&m_writeBuffer};
        bufferStream.write(data, size);
        if (!m_isWritting) {
            return doWrite();
        }
    }
    else{
        std::cerr << "Socket is closed.\n";
        return false;
    }
    return false;
}

bool TcpConnection::doWrite() {
    m_isWritting = true;
    auto self = shared_from_this();
    boost::system::error_code error;
    auto bytesTransferred = m_socket.write_some(m_writeBuffer.data(), error);
      if (error) {
        std::cerr << "TcpConnection::doWrite() error: " + error.message() + ".\n";
        return false;
      }
      m_writeBuffer.consume(bytesTransferred);
      if (m_writeBuffer.size() == 0) {
        m_isWritting = false;
        return true;
      }
      return doWrite();
  }

void TcpConnection::close(){
    if(m_socket.is_open()){
        try {
            m_socket.close();
        } catch (const std::exception &e) {
            std::cerr << "TcpConnection::close() exception: " +
                            static_cast<std::string>(e.what()) + ".\n";
            return;
        }
    }
    m_object.onClose(m_connectionId);
}

std::shared_ptr<TcpConnection> TcpConnection::create(tcp::socket &&socket, TcpObject &object, int connId){
    return std::shared_ptr<TcpConnection>(
        new TcpConnection{std::move(socket), object, connId});
}
