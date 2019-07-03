# Ubuntu 18.04
FROM phusion/baseimage:0.11 

LABEL maintainer="gabriel.figueiredo@imperial.ac.uk"

RUN apt-get update && \
    apt-get install -y \
    automake \
    bison \
    ca-certificates \
    curl \
    flex \
    g++ \
    gcc \
    ghostscript \
    git \
    libpython3-dev \
    libtool \
    make \
    nano \
    pkg-config \
    python3 \
    python3-pip \
    software-properties-common \
    tree \
    unzip \
    wget 

# install gcc-4.9 for ROSE
# RUN  for f in "cpp-4.9_4.9.4"  \
#               "g++-4.9_4.9.4" \
#               "gcc-4.9_4.9.4" \
#               "gcc-4.9-base_4.9.4" \
#               "libasan1_4.9.4" \
#               "libgcc-4.9-dev_4.9.4" \
#               "libstdc++-4.9-dev_4.9.4";  do \
#          wget -P /tmp https://launchpad.net/~ubuntu-toolchain-r/+archive/ubuntu/test/+build/10707174/+files/$f-2ubuntu1~16.04_amd64.deb;    \   
#        done  && \
#        wget -P /tmp http://mirrors.kernel.org/ubuntu/pool/main/m/mpfr4/libmpfr4_3.1.4-1_amd64.deb &&  \
#        apt install -y /tmp/libmpfr4_3.1.4-1_amd64.deb \
#             /tmp/gcc-4.9-base_4.9.4-2ubuntu1~16.04_amd64.deb \
#             /tmp/libasan1_4.9.4-2ubuntu1~16.04_amd64.deb  \
#             /tmp/libgcc-4.9-dev_4.9.4-2ubuntu1~16.04_amd64.deb  \
#             /tmp/cpp-4.9_4.9.4-2ubuntu1~16.04_amd64.deb  \
#             /tmp/gcc-4.9_4.9.4-2ubuntu1~16.04_amd64.deb  \
#             /tmp/libstdc++-4.9-dev_4.9.4-2ubuntu1~16.04_amd64.deb \
#             /tmp/g++-4.9_4.9.4-2ubuntu1~16.04_amd64.deb 
      

RUN  pip3 install --upgrade pip ansicolors setuptools wheel gitpython prettytable Pygments pillow 

COPY runtime-dev /usr/local/bin

RUN chmod +x /usr/local/bin/runtime-dev && echo "root:root" | chpasswd
# Use baseimage-docker's init system.
CMD ["/sbin/my_init"]
VOLUME "/workspace"  "artisan"
# Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

