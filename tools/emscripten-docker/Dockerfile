FROM trzeci/emscripten:sdk-incoming-64bit

MAINTAINER Gerard Choinka <gerard.choinka@gmail.com>


RUN wget -qO- "https://cmake.org/files/v3.12/cmake-3.12.1-Linux-x86_64.tar.gz" | tar xz -C /opt/

RUN echo "deb http://ftp.us.debian.org/debian testing main contrib non-free" >> /etc/apt/sources.list && \
    apt-get -y update && \
    apt-get -y -t testing install gcc-7
