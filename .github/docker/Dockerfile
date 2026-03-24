FROM debian:bookworm-slim

# Avoid tzdata prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install only the strict minimum to compile for ARM (avoids downloading hundreds of MBs)
RUN apt-get update && apt-get install -y --no-install-recommends \
    make \
    g++-arm-linux-gnueabihf \
    && rm -rf /var/lib/apt/lists/*

# Set the cross-compiler as default for Makefile
ENV CXX=arm-linux-gnueabihf-g++
ENV CC=arm-linux-gnueabihf-gcc

WORKDIR /app
