#include <gtest/gtest.h>
#include "tcp_server.hpp"
#include "mock_tcp_client.hpp"
#include <bits/this_thread_sleep.h>

using ::testing::StrictMock;

TEST(TcpServerClientTest, BasicConnection) {
    boost::asio::io_context io_context;
    TcpServer server(12345, io_context);
    server.start();

    MockTcpClient client(io_context);
    std::thread thread{[&io_context]() { io_context.run(); }};
    std::string command = "CONNECT 12345 client1";
    client.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(client.isConnected());
    ASSERT_EQ(server.getClientCount(), 1);
    ASSERT_EQ(server.getClientName(0), "client1");

    command = "DISCONNECT";
    client.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(client.isConnected());
    ASSERT_EQ(server.getClientCount(), 0);
    ASSERT_EQ(server.getClientName(0), "");

    io_context.stop();
    thread.join();
}

TEST(TcpServerClientTest, TwoClientConnections) {
    boost::asio::io_context io_context;
    TcpServer server(12345, io_context);
    server.start();

    MockTcpClient client1(io_context);
    MockTcpClient client2(io_context);
    std::thread thread{[&io_context]() { io_context.run(); }};

    std::string command = "CONNECT 12345 client1";
    client1.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(client1.isConnected());
    ASSERT_EQ(server.getClientCount(), 1);
    ASSERT_EQ(server.getClientName(0), "client1");

    command = "CONNECT 12345 client2";
    client2.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(client2.isConnected());
    ASSERT_EQ(server.getClientCount(), 2);
    ASSERT_EQ(server.getClientName(1), "client2");

    command = "DISCONNECT";
    client1.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(client1.isConnected());
    ASSERT_EQ(server.getClientCount(), 1);
    ASSERT_EQ(server.getClientName(0), "");

    client2.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(client2.isConnected());
    ASSERT_EQ(server.getClientCount(), 0);
    ASSERT_EQ(server.getClientName(1), "");

    io_context.stop();
    thread.join();
}

TEST(TcpServerClientTest, OneClientSubscribePublish) {
    boost::asio::io_context io_context;
    TcpServer server(12345, io_context);
    server.start();

    // Using StrickMock so that we can check if onRead is called only when expected
    StrictMock<MockTcpClient> client(io_context);
    std::thread thread{[&io_context]() { io_context.run(); }};

    std::string command = "CONNECT 12345 client1";
    client.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(client.isConnected());
    ASSERT_EQ(server.getClientCount(), 1);
    ASSERT_EQ(server.getClientName(0), "client1");

    command = "SUBSCRIBE test";
    client.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    command = "PUBLISH test test";
    EXPECT_CALL(client, onRead(0, "test;test")).Times(1);
    client.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Should not call onRead because client is not subscribed to test1
    command = "PUBLISH test1 test";
    client.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    command = "DISCONNECT";
    client.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(client.isConnected());
    ASSERT_EQ(server.getClientCount(), 0);
    ASSERT_EQ(server.getClientName(0), "");

    io_context.stop();
    thread.join();
}

TEST(TcpServerClientTest, TwoClientsSubscribePublish) {
    boost::asio::io_context io_context;
    TcpServer server(12345, io_context);
    server.start();

    // Using StrickMock so that we can check if onRead is called only when expected
    StrictMock<MockTcpClient> client(io_context);
    StrictMock<MockTcpClient> client2(io_context);
    std::thread thread{[&io_context]() { io_context.run(); }};

    std::string command = "CONNECT 12345 client1";
    client.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(client.isConnected());
    ASSERT_EQ(server.getClientCount(), 1);
    ASSERT_EQ(server.getClientName(0), "client1");

    command = "CONNECT 12345 client2";
    client2.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(client2.isConnected());
    ASSERT_EQ(server.getClientCount(), 2);
    ASSERT_EQ(server.getClientName(1), "client2");

    command = "SUBSCRIBE test";
    client.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    command = "SUBSCRIBE test1";
    client.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    command = "SUBSCRIBE test";
    client2.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    command = "PUBLISH test test";
    EXPECT_CALL(client, onRead(0, "test;test")).Times(1);
    EXPECT_CALL(client2, onRead(0, "test;test")).Times(1);
    client.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Should not call onRead because client is not subscribed to test1
    command = "PUBLISH test1 test";
    EXPECT_CALL(client, onRead(0, "test1;test")).Times(1);
    client2.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    command = "DISCONNECT";
    client.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(client.isConnected());
    ASSERT_EQ(server.getClientCount(), 1);
    ASSERT_EQ(server.getClientName(0), "");

    client2.handleCommand(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(client2.isConnected());
    ASSERT_EQ(server.getClientCount(), 0);
    ASSERT_EQ(server.getClientName(1), "");


    io_context.stop();
    thread.join();
}

TEST(TcpServerClientTest, MaxNoOfClients) {
    boost::asio::io_context io_context;
    TcpServer server(12345, io_context);
    server.start();
    std::thread thread{[&io_context]() { io_context.run(); }};
    std::vector<std::unique_ptr<StrictMock<MockTcpClient>>> clients;
    
    for (int i = 0; i < Constants::max_clients; ++i) {
        clients.push_back(std::make_unique<StrictMock<MockTcpClient>>(io_context));
    }

    for (int i = 0; i < Constants::max_clients; ++i) {
        std::string command = "CONNECT 12345 client" + std::to_string(i + 1);
        clients[i]->handleCommand(command);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ASSERT_TRUE(clients[i]->isConnected());
        ASSERT_EQ(server.getClientCount(), i + 1);
        ASSERT_EQ(server.getClientName(i), "client" + std::to_string(i + 1));
    }

    for (int i = 0; i < Constants::max_clients; ++i) {
        std::string command = "SUBSCRIBE test";
        clients[i]->handleCommand(command);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    for (int i = 0; i < Constants::max_clients; ++i) {
        EXPECT_CALL(*clients[i], onRead(0, "test;test")).Times(1);
    }

    // One client send a message to a topic everyone is subscribed to
    std::string command = "PUBLISH test test";
    clients[0]->handleCommand(command);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (int i = 0; i < Constants::max_clients; ++i) {
        std::string command = "DISCONNECT";
        clients[i]->handleCommand(command);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ASSERT_FALSE(clients[i]->isConnected());
        ASSERT_EQ(server.getClientCount(), Constants::max_clients - 1 - i);
        ASSERT_EQ(server.getClientName(i), "");
    }

    io_context.stop();
    thread.join();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}