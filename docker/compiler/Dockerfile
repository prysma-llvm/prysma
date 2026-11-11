FROM ubuntu:24.04 AS base

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    ca-certificates \
    curl \
    time \
    clang-18 \
    llvm-18 \
    llvm-18-dev \
    lld-18 \
    libclang-rt-18-dev \
    libzstd-dev \
    zlib1g-dev \
    libedit-dev \
    libcurl4-openssl-dev \
    ccache \
    python3 \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

RUN ln -sf /usr/bin/clang-18 /usr/bin/clang && \
    ln -sf /usr/bin/clang++-18 /usr/bin/clang++

RUN python3 -m venv /opt/venv

ENV LLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm
ENV PATH="/usr/lib/llvm-18/bin:/opt/venv/bin:$PATH"
ENV CCACHE_DIR=/tmp/.ccache

COPY requirements.txt /tmp/requirements.txt
RUN pip install --no-cache-dir -r /tmp/requirements.txt

WORKDIR /workspace

# Développement et débogage
FROM base AS debug

RUN apt-get update && apt-get install -y --no-install-recommends \
    gdb \
    lldb-18 \
    bear \
    graphviz \
    && rm -rf /var/lib/apt/lists/*

RUN pip install --no-cache-dir debugpy

CMD ["/bin/bash"]

# Serveur de benchmarks
FROM base AS benchmark

RUN apt-get update && apt-get install -y --no-install-recommends \
    linux-tools-common \
    linux-tools-generic \
    && rm -rf /var/lib/apt/lists/*

COPY docker/compiler/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
