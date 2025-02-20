FROM alpine:3.19 AS build

# Set environment variables to avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies (build-essential, cmake, pthreads, etc.)
RUN apk --update add --no-cache \
  build-base \
  cmake \
  boost boost-dev

# Create a directory for the project and copy the source code
WORKDIR /app

COPY inc/ /app/inc/
COPY src/ /app/src/
COPY CMakeLists.txt /app/

RUN cmake -S . -B build && cmake --build build