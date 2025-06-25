FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    iputils-ping && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake . && make

CMD ["./AR_UDP_Receiver"]