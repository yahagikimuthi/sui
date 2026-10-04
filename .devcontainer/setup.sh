#!/bin/bash
set -e

# ユーザーグループの生成
sudo groupadd -r systemd-journal 2>/dev/null || true
sudo useradd -r -s /sbin/nologin systemd-network 2>/dev/null || true

sudo apt-get update
sudo apt-get install -y software-properties-common wget gpg ca-certificates

# Ubuntu Toolchain PPA (GCC 16用)
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y

# Kitware Official Repository (最新 CMake用)
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null

sudo apt-get install -y \
    gcc-16 \
    g++-16 \
    cmake \
    ninja-build \
    gdb \
    lldb \
    libvulkan-dev \
    vulkan-tools \
    vulkan-validationlayers \
    vulkan-utility-libraries-dev \
    spirv-tools \
    glslang-tools \
    glslang-dev \
    glslc \
    mesa-vulkan-drivers \
    libglfw3-dev \
    libglm-dev \
    pkg-config

sudo add-apt-repository ppa:kisak/kisak-mesa
sudo apt update
sudo apt upgrade

echo "=== sui devcontainer setup complete ==="