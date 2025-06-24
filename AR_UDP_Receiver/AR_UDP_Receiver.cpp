#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <set>
#include <string>

int extract_sequence(const std::string& msg) {
    int seq = -1;
    size_t start = msg.find("SEQ:");
    size_t end = msg.find("|", start);
    if (start != std::string::npos && end != std::string::npos) {
        std::string seq_str = msg.substr(start + 4, end - (start + 4));
        try {
            seq = std::stoi(seq_str);
        } catch (...) {
            return -1;
        }
    }
    return seq;
}



int main() 
{
    int ar_sockfd;
    struct sockaddr_in ar_server_addr, ar_client_addr;
    char buffer[256];
    socklen_t ar_addr_len = sizeof(ar_client_addr);

    std::set<int> ar_received_sequences;
    int ar_expected_seq = 1;


    ar_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ar_sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&ar_server_addr, 0, sizeof(ar_server_addr));
    ar_server_addr.sin_family = AF_INET;
    ar_server_addr.sin_port = htons(8080);
    ar_server_addr.sin_addr.s_addr = INADDR_ANY;


    if (bind(ar_sockfd, (const struct sockaddr*)&ar_server_addr, sizeof(ar_server_addr)) < 0) 
    {
        perror("Bind failed");
        close(ar_sockfd);
        return 1;
    }

    
    std::cout << "UDP Receiver listening on port 8080...\n";
    
    

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t len = recvfrom(ar_sockfd, buffer, sizeof(buffer) - 1, 0,
                               (struct sockaddr*)&ar_client_addr, &ar_addr_len);
        if (len < 0) {
            perror("recvfrom failed");
            break;
        }


        buffer[len] = '\0';
        std::string msg(buffer);

        if (msg == "STOP") {
            std::cout << "\nReceived STOP. Shutting down receiver.\n";
            break;
        }

        std::cout << "\nReceived: " << msg << std::endl;

        int seq = extract_sequence(msg);
        if (seq == -1) {
            std::cout << "Invalid message format. Skipping...\n";
            continue;
        }

        if (ar_received_sequences.count(seq)) {
            std::cout << "Duplicate packet detected: SEQ:" << seq << "\n";
            continue;
        }

        if (seq > ar_expected_seq) {
            std::cout << "Packet loss detected! Missing SEQ:" << ar_expected_seq << " to SEQ:" << (seq - 1) << "\n";
        }

        if (seq < ar_expected_seq) {
            std::cout << "Out-of-order packet detected: SEQ:" << seq << " (expected: " << ar_expected_seq << ")\n";
        }

        ar_received_sequences.insert(seq);
        if (seq >= ar_expected_seq)
            ar_expected_seq = seq + 1;
    }

    close(ar_sockfd);
    return 0;


}

