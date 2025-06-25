#include <iostream>
#include <unistd.h>     //For posix standard functions.
#include <arpa/inet.h>                    // For inet_addr, htons, sockaddr_in, etc.
#include <cstring>                      //for memset() , memcpy().
#include <string>          //string library
#include <netdb.h>       //search the host by docker service name(to resolve hostnames)

int main() 
{

    int ar_sockfd;  // Socket file descriptor as a part of posix standard
    struct sockaddr_in ar_receiver_addr; // store the receiver address
    ar_sockfd = socket(AF_INET, SOCK_DGRAM, 0); //socket number assigned after udp socket is created

    if (ar_sockfd < 0) { //check whether the udp socket is created or not.
        perror("Socket creation failed");
        return 1;
    }


    memset(&ar_receiver_addr, 0, sizeof(ar_receiver_addr)); //filling zeros to address structure
    ar_receiver_addr.sin_family = AF_INET; // to use IPV4 standard
    ar_receiver_addr.sin_port = htons(8080); // set the port number

 // Use Docker service name as IP address
    //ar_receiver_addr.sin_addr.s_addr = inet_addr("udp_receiver");

    struct hostent *he = gethostbyname("udp_receiver"); // to find the IP address of the mentioned docker service name
    if (he == NULL) { //check whether the IP address resolution is done successfully or not..
        perror("gethostbyname failed");
        exit(1);
    }
    memcpy(&ar_receiver_addr.sin_addr, he->h_addr_list[0], he->h_length); //copy the resolved address to the structure

    std::string ar_input_seq, ar_input_msg; // To store the input sequence number and input message . SEquence number is the number to check the message correctness.

    while (true) 
    {
        if (!std::getline(std::cin, ar_input_seq)) break; // Read Sequence number from the input console

        if (ar_input_seq == "STOP") {  // STOP the communication if the sequence is received as STOP.
            std::string stop_message = "STOP";
            sendto(ar_sockfd, stop_message.c_str(), stop_message.length(), 0,
                   (const struct sockaddr*)&ar_receiver_addr, sizeof(ar_receiver_addr));
            break;
        }
        if (!std::getline(std::cin, ar_input_msg)) break; // Read the actual message from the standard input console

        std::string full_message = "SEQ:" + ar_input_seq + "|MSG:" + ar_input_msg; //convert the message into SEQUENCE+MESSAGE format

        sendto(ar_sockfd, full_message.c_str(), full_message.length(), 0,
               (const struct sockaddr*)&ar_receiver_addr, sizeof(ar_receiver_addr));

        std::cout << "Sent: " << full_message << std::endl;  // printing that the message was sent on console.
    }

    close(ar_sockfd); // close the socket and release the system resource
    return 0; //EXIT
}


