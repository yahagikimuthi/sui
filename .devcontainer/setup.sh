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

sudo chmod 666 /dev/dri/card* /dev/dri/renderD* 2>/dev/null || true

sudo apt-get update && sudo apt-get install -y \
    gcc-16 \
    g++-16 \
    cmake \
    ninja-build \
    gdb \
    lldb \
    libvulkan-dev \
    vulkan-tools \
    vulkan-validationlayers \
    libglfw3-dev \
    libx11-xcb-dev \
    libxcb1-dev \
    libasound2-dev \
    pkg-config \
    mesa-vulkan-drivers \
    mesa-utils

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100 \
  --slave /usr/bin/g++ g++ /usr/bin/g++-16

sudo apt update
sudo apt upgrade -y

echo 'export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/intel_icd.json' >> ~/.bashrc

echo "=== sui devcontainer setup complete ==="