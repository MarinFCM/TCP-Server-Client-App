# TCP Server-Client Application

This repository contains a TCP server-client application implemented using Boost.Asio. 
The server can handle multiple clients, and clients can connect, disconnect, subscribe/unsubscribe to topics, and publish messages to topics. 
The repository also includes unit tests using Google Test and Google Mock.

## Features

- TCP server that can handle multiple clients
- Clients can connect and disconnect
- Clients can subscribe/unsubscribe to topics
- Clients can publish messages to topics
- Unit tests using Google Test and Google Mock

## Installation

### Prerequisites

- Docker
- CMake (if building locally)
- Boost (if building locally)
- Google Test (if building locally)

### Building the Docker Image

To build the Docker image, run the following command:

```sh
docker build -t tcp_app .
```

### Running the Server and Client

To run the server and client using Docker, use the following commands:

```sh
# Run the server on port 1999
docker run -it --network host --rm tcp_app build/tcp_server 1999

# Run the client
docker run -it --network host --rm tcp_app build/tcp_client
```

### Running the Tests

To run the unit tests using Docker, use the following command:

```sh
docker run -it --rm tcp_app build/test/TCP-Server-Client-Test
```

## Directory Structure

```
├── src
│   ├── tcp_client.cpp
│   ├── tcp_connection.cpp
│   ├── tcp_server.cpp
├── inc
│   ├── tcp_client.hpp
│   ├── tcp_connection.hpp
│   ├── tcp_server.hpp
├── test
│   ├── CMakeLists.txt
│   ├── tests.cpp
│   ├── mock_tcp_client.hpp
├── CMakeLists.txt
├── Dockerfile
└── README.md
```

## Building and Running Locally

If you prefer to build and run the application locally without Docker, follow these steps:

### Prerequisites

- CMake
- Boost.Asio
- Google Test and Google Mock

### Building the Application

1. Install dependencies:
    ```sh
    sudo apt-get install libboost-dev
    sudo apt-get install libgtest-dev
    ```

2. Run CMake to configure the project:

    ```sh
    cmake ..
    ```

3. Build the project:

    ```sh
    cmake -S . -B build
    cmake --build build
    ```

### Running the Server and Client

1. Run the server:

    ```sh
    ./build/tcp_server 1999
    ```

2. Run the client:

    ```sh
    ./build/tcp_client
    ```

### Running the Tests

To run the unit tests, use the following command:

```sh
./build/test/TCP-Server-Client-Test
```

## Contributing

Contributions are welcome! Please open an issue or submit a pull request.

## License

This project is licensed under the MIT License.