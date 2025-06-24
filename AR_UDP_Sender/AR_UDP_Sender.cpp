#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <netdb.h>

int main() 
{

    int ar_sockfd;
    struct sockaddr_in ar_receiver_addr;


    ar_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (ar_sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }


    memset(&ar_receiver_addr, 0, sizeof(ar_receiver_addr));
    ar_receiver_addr.sin_family = AF_INET;
    ar_receiver_addr.sin_port = htons(8080);

 // Use Docker service name as IP address
    //ar_receiver_addr.sin_addr.s_addr = inet_addr("udp_receiver");

    struct hostent *he = gethostbyname("udp_receiver");
    if (he == NULL) {
        perror("gethostbyname failed");
        exit(1);
    }
    memcpy(&ar_receiver_addr.sin_addr, he->h_addr_list[0], he->h_length);

    std::string ar_input_seq, ar_input_msg;

    while (true) 
    {
        if (!std::getline(std::cin, ar_input_seq)) break;

        if (ar_input_seq == "STOP") {
            std::string stop_message = "STOP";
            sendto(ar_sockfd, stop_message.c_str(), stop_message.length(), 0,
                   (const struct sockaddr*)&ar_receiver_addr, sizeof(ar_receiver_addr));
            break;
        }
        if (!std::getline(std::cin, ar_input_msg)) break;

        std::string full_message = "SEQ:" + ar_input_seq + "|MSG:" + ar_input_msg;

        sendto(ar_sockfd, full_message.c_str(), full_message.length(), 0,
               (const struct sockaddr*)&ar_receiver_addr, sizeof(ar_receiver_addr));

        std::cout << "Sent: " << full_message << std::endl;
    }

    close(ar_sockfd);
    return 0;
}


