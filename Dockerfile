# Start from your base image
FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update system and install essential tools (Git, build tools, curl, etc.)
RUN apt-get update && apt-get install -y \
    git \
    curl \
    build-essential \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js and npm (required for OpenSpec and OpenCode)
# Install prerequisites, set up a stable LTS version of Node.js (Node 20), and install it
RUN apt-get update && apt-get install -y \
    curl \
    && curl -fsSL https://deb.nodesource.com/setup_lts.x | bash - \
    && apt-get install -y nodejs \
    && rm -rf /var/lib/apt/lists/
# Globally install OpenSpec and OpenCode so they are permanently baked into the image
RUN npm install -g @fission-ai/openspec@latest

# (Assuming open-code is available via npm; adjust package name if it uses a different registry/scoped package)
RUN curl -fsSL https://opencode.ai/install | bash

# Set your default working directory inside the container
WORKDIR /workspace
