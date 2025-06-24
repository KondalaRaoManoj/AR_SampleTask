FROM gcc:latest

WORKDIR /app

COPY AR_UDP_Receiver.cpp .

RUN g++ -o ar_udp_receiver AR_UDP_Receiver.cpp

CMD ["./ar_udp_receiver"]

