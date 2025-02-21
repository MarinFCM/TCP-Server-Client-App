#include <gmock/gmock.h>
#include "tcp_client.hpp"

class MockTcpClient : public TcpClient {
public:
    MockTcpClient(boost::asio::io_context& io_context)
        : TcpClient(io_context) {}

    // A mock class is needed only for onRead because it doesn't return a value 
    // but we still want to check if the correct message was received
    MOCK_METHOD(void, onRead, (int connId, std::string payload), (override));
};