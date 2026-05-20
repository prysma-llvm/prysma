FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    clang-18 \
    llvm-18 \
    llvm-18-dev \
    lld-18 \
    libzstd-dev \
    libedit-dev \
    libcurl4-openssl-dev \
    bear \
    graphviz \
    python3 \
    python3-pip \
    python3-venv \
    linux-tools-common \
    linux-tools-generic \
    && rm -rf /var/lib/apt/lists/*

RUN ln -sf /usr/bin/clang-18 /usr/bin/clang && \
    ln -sf /usr/bin/clang++-18 /usr/bin/clang++

WORKDIR /workspace

RUN python3 -m venv /opt/venv
ENV LLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm
ENV PATH="/usr/lib/llvm-18/bin:/opt/venv/bin:$PATH"

RUN pip install --no-cache-dir Jinja2 PyYAML

CMD ["python3", "tests/run_perf_tests.py"]
