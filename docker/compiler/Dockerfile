# Ubuntu 24.04, linux/amd64 image manifest (not a mutable tag).
FROM --platform=linux/amd64 ubuntu:24.04@sha256:9a04f13b3f80ef056361e30868593b1cfd2279fe19e4081cb93a90bf56819d6a AS base

ARG DEBIAN_FRONTEND=noninteractive

# The minimal Ubuntu base has no CA bundle. Bootstrap HTTPS with the
# checksum-verified CA package from the same fixed, signed Ubuntu archive.
ADD https://snapshot.ubuntu.com/ubuntu/20261127T000000Z/pool/main/c/ca-certificates/ca-certificates_20260816ubuntu0.24.04.2_all.deb /tmp/snapshot-ca.deb
COPY docker/compiler/ubuntu-snapshot.sources /etc/apt/sources.list.d/ubuntu.sources
RUN echo 'f9fbacd417b515450b831032cc39d18059e9874e67525679a43c16071dabf9a0  /tmp/snapshot-ca.deb' | sha256sum -c - \
    && dpkg-deb -x /tmp/snapshot-ca.deb /tmp/snapshot-ca \
    && find /tmp/snapshot-ca/usr/share/ca-certificates -name '*.crt' -type f -exec cat {} + > /tmp/snapshot-ca.pem \
    && test -s /tmp/snapshot-ca.pem \
    && apt-get -o Acquire::https::CaInfo=/tmp/snapshot-ca.pem -o APT::Update::Error-Mode=any update \
    && apt-get -o Acquire::https::CaInfo=/tmp/snapshot-ca.pem install -y --no-install-recommends \
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
    && rm -rf /var/lib/apt/lists/* /tmp/snapshot-ca /tmp/snapshot-ca.deb /tmp/snapshot-ca.pem

RUN ln -sf /usr/bin/clang-18 /usr/bin/clang && \
    ln -sf /usr/bin/clang++-18 /usr/bin/clang++

RUN python3 -m venv /opt/venv

ENV LLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm
ENV PATH="/usr/lib/llvm-18/bin:/opt/venv/bin:$PATH"
ENV CCACHE_DIR=/tmp/.ccache

COPY docker/compiler/requirements.lock /tmp/requirements.lock
RUN python -m pip --isolated install --index-url https://pypi.org/simple \
    --no-cache-dir --no-compile --require-hashes --only-binary=:all: \
    -r /tmp/requirements.lock

WORKDIR /workspace

# Développement et débogage
FROM base AS debug

RUN apt-get -o APT::Update::Error-Mode=any update && apt-get install -y --no-install-recommends \
    gdb \
    lldb-18 \
    bear \
    graphviz \
    && rm -rf /var/lib/apt/lists/*

CMD ["/bin/bash"]

# Serveur de benchmarks
FROM base AS benchmark

RUN apt-get -o APT::Update::Error-Mode=any update && apt-get install -y --no-install-recommends \
    linux-tools-common \
    linux-tools-generic \
    && rm -rf /var/lib/apt/lists/*

COPY docker/compiler/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
