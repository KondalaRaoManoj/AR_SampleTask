#include <iostream>
#include <cstring>//for memset() , memcpy().
#include <unistd.h>//For posix standard functions.
#include <arpa/inet.h>// For inet_addr, htons, sockaddr_in, etc.
#include <set> // to track and check the sequence numbers for uniqueness
#include <string>//string library


//This function is defined to extract the received sequencenumber. 
int extract_sequence(const std::string& msg) {
    int seq = -1;     //considered as default  invalid sequence number
    size_t start = msg.find("SEQ:");    //received sequence starting
    size_t end = msg.find("|", start);  // Seperator between sequence and the message.
    if (start != std::string::npos && end != std::string::npos) {
        std::string seq_str = msg.substr(start + 4, end - (start + 4)); //Extract the sequence number part 
        try {
            seq = std::stoi(seq_str); //convert string to integer
        } catch (...) {
            return -1;  //return -1 if conversion fails as an invalid sequence number
        }
    }
    return seq; //returns the sequence number after extraction
}



int main() 
{
    int ar_sockfd;// Sock descriptor as a part of posix standard
    struct sockaddr_in ar_server_addr, ar_client_addr;// store the receiver address and sender address
    char buffer[256];// Buffer to store incoming messages
    socklen_t ar_addr_len = sizeof(ar_client_addr);// Length of client address struct

    std::set<int> ar_received_sequences; // To store unique received sequence numbers
    int ar_expected_seq = 1;  // Start expecting from sequence 1


    ar_sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Create a UDP socket
    if (ar_sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&ar_server_addr, 0, sizeof(ar_server_addr));//filling zeros to address structure
    ar_server_addr.sin_family = AF_INET;// to use IPV4 standard
    ar_server_addr.sin_port = htons(8080);// set the port number
    ar_server_addr.sin_addr.s_addr = INADDR_ANY;// Accept packets from any IP


    if (bind(ar_sockfd, (const struct sockaddr*)&ar_server_addr, sizeof(ar_server_addr)) < 0) // the POSIX call to listen on port 8080 and receive udp packets
    {
        perror("Bind failed");
        close(ar_sockfd);
        return 1;
    }

    
    std::cout << "UDP Receiver listening on port 8080...\n";
    
    

    while (true) {
        memset(buffer, 0, sizeof(buffer));//clear buffer
        ssize_t len = recvfrom(ar_sockfd, buffer, sizeof(buffer) - 1, 0,
                               (struct sockaddr*)&ar_client_addr, &ar_addr_len);// To receive the message
        if (len < 0) {
            perror("recvfrom failed");
            break;
        }


        buffer[len] = '\0'; // Terminate the last element after the message
        std::string msg(buffer); // to convert to a string

        if (msg == "STOP") {//check for end of communication
            std::cout << "\nReceived STOP. Shutting down receiver.\n";
            break;
        }

        std::cout << "\nReceived: " << msg << std::endl;

        int seq = extract_sequence(msg);// to extract message sequence
        if (seq == -1) {
            std::cout << "Invalid message format. Skipping...\n";
            continue;
        }

        if (ar_received_sequences.count(seq)) { // check for duplicate sequences
            std::cout << "Duplicate packet detected: SEQ:" << seq << "\n";
            continue;
        }

        if (seq > ar_expected_seq) { //detection of mission packets
            std::cout << "Packet loss detected! Missing SEQ:" << ar_expected_seq << " to SEQ:" << (seq - 1) << "\n";
        }

        if (seq < ar_expected_seq) { // sequence out of order ...
            std::cout << "Out-of-order packet detected: SEQ:" << seq << " (expected: " << ar_expected_seq << ")\n";
        }

        ar_received_sequences.insert(seq);// Sequence is received....and stored in set...used for further monitorings...
            ar_expected_seq = seq + 1; //next expected sequence is updated...
    }

    close(ar_sockfd); //close the socket...
    return 0;


}

