FROM debian:stable-slim

RUN apt-get update && apt-get install -y \
    systemd \
    curl \
    && curl -fsSL https://get.docker.com | sh \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

COPY lab_mode.sh /usr/local/bin/lab_mode.sh
RUN chmod +x /usr/local/bin/lab_mode.sh

ENTRYPOINT ["/usr/local/bin/lab_mode.sh"]
