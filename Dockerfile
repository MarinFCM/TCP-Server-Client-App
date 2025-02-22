FROM alpine:3.19 AS build

# Install the necessary packages
RUN apk --update add --no-cache \
  build-base \
  cmake \
  boost boost-dev \
  gtest-dev

# Create a directory for the project and copy the source code
WORKDIR /app

COPY inc/ /app/inc/
COPY src/ /app/src/
COPY test/ /app/test/
COPY CMakeLists.txt /app/

# Build the project
RUN cmake -S . -B build && cmake --build build