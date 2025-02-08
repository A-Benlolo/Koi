######################################
# Use Ubuntu 20.04 as the base image #
######################################
FROM ubuntu:20.04 AS base
ARG DEBIAN_FRONTEND=noninteractive
USER root
WORKDIR /

ARG USERNAME=nereus
ARG TRITON_ROOT=/Triton
ARG TRITON_BUILD=/tmp/triton-build
ARG BASHRC=/etc/bash.bashrc

# Install build and development tools
RUN apt update && \
    apt upgrade && \
    apt install -y build-essential \
                   clang \
                   curl \
                   git \
                   libboost-all-dev \
                   libgmp-dev \
                   libpython3-dev \
                   libpython3-stdlib \
                   llvm-12 \
                   llvm-12-dev \
                   python3-pip \
                   sudo \
                   tar \
                   ninja-build \
                   pkg-config  \
                   vim && \
    apt-get clean && \
    pip install --upgrade pip && \
    pip3 install Cython lief cmake meson

# Install libcapstone >= 5.0.x
RUN cd /tmp && \
    curl -o cap.tgz -L https://github.com/aquynh/capstone/archive/5.0.1.tar.gz && \
    tar xvf cap.tgz && cd capstone-5.0.1/ && CAPSTONE_ARCHS="arm aarch64 riscv x86" ./make.sh && \
    make install && rm -rf /tmp/cap* \
    && ln -s /usr/lib/libcapstone.so.5 /usr/lib/x86_64-linux-gnu/libcaps

# Install libbitwuzla >= 0.4.0
RUN cd /tmp && \
    git clone https://github.com/bitwuzla/bitwuzla.git && \
    cd bitwuzla && \
    git checkout -b 0.4.0 0.4.0 && \
    python3 ./configure.py --shared && \
    cd build && \
    ninja install && \
    ldconfig

# Install libz3 >= 4.6.0
RUN pip3 install z3-solver==4.8.14

# Build Triton binaries (LLVM for lifting; z3 or bitwuzla as SMT solver)
RUN PYV=`python3 -c "import platform;print(platform.python_version()[:3])"` && \
    git clone https://github.com/JonathanSalwan/Triton.git $TRITON_ROOT && \
    mkdir -p $TRITON_BUILD && \
    cd $TRITON_BUILD && \
    cmake -DLLVM_INTERFACE=ON \
          -DCMAKE_PREFIX_PATH=$(/usr/lib/llvm-12/bin/llvm-config --prefix) \
          -DZ3_INTERFACE=ON \
          -DZ3_INCLUDE_DIRS=/usr/local/lib/python$PYV/dist-packages/z3/include/ \
          -DZ3_LIBRARIES=/usr/local/lib/python$PYV/dist-packages/z3/lib/libz3.so \
          -DBITWUZLA_INTERFACE=ON \
          -DBITWUZLA_INCLUDE_DIRS=/usr/local/include \
          -DBITWUZLA_LIBRARIES=/usr/local/lib/x86_64-linux-gnu/libbitwuzla.so \
          $TRITON_ROOT && \
    make -j$(nproc) && \
    make install

# Build Triton Python package
RUN PYV=`python3 -c "import platform;print(platform.python_version()[:3])"` && \
    PYP="/usr/lib/python$PYV/site-packages" && \
    echo export PYTHONPATH="$PYP:\$PYTHONPATH" >> $BASHRC && \
    python3 -c "import z3; print('Z3 version:', z3.get_version_string())" && \
    PYTHONPATH="$PYP" python3 -c "from triton import *; ctx=TritonContext(ARCH.X86_64); ctx.setSolver(SOLVER.Z3); ctx.setSolver(SOLVER.BITWUZLA);"

# Create a new user
RUN useradd -d /home/$USERNAME -m $USERNAME -s /bin/bash && \
    passwd -d $USERNAME && \
    usermod -aG sudo $USERNAME

# Move Triton to user home
RUN mv $TRITON_ROOT /home/$USERNAME/ && \
    chown -R 1000:1000 /home/$USERNAME

# Build and install Koi
RUN cd /home/$USERNAME && \
    git clone https://www.github.com/A-Benlolo/Koi.git Koi && \
    cd Koi && \
    make dev && \
    echo "export LD_LIBRARY_PATH=~/usr/local/lib:\$LD_LIBRARY_PATH" >> ~/.bashrc


##################################################
# Copy everything from base into a scratch image #
# This reduces the image to a single layer       #
##################################################
FROM scratch
ARG USERNAME=nereus
COPY --from=base / /
USER $USERNAME
WORKDIR /home/$USERNAME/Koi
ENTRYPOINT ["/bin/bash"]
