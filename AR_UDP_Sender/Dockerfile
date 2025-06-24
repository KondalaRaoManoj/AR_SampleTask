FROM gcc:latest

WORKDIR /app

COPY AR_UDP_Sender.cpp .

RUN g++ -o ar_udp_sender AR_UDP_Sender.cpp

CMD ["./ar_udp_sender"]