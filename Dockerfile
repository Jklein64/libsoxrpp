FROM ubuntu:20.04

# Never prompts the user for choices on installation/configuration of packages
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=America/New_York
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en

# Install basic dev dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    git \
    libtool \
    libssl-dev \
    lsb-release \
    software-properties-common \
    unzip \
    wget

# Install CMake. See https://askubuntu.com/a/865294
RUN /bin/bash <<EOF
    set -euxo pipefail
    apt-get remove --purge --auto-remove cmake
    test -f /usr/share/doc/kitware-archive-keyring/copyright || wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null
    apt-get update
    test -f /usr/share/doc/kitware-archive-keyring/copyright || rm /usr/share/keyrings/kitware-archive-keyring.gpg
    apt-get install -y --no-install-recommends kitware-archive-keyring
    apt-get update
    apt-get install -y --no-install-recommends cmake
EOF

WORKDIR /soxrpp
COPY . .
