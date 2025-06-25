#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <arpa/inet.h>
#include <netdb.h>
#include <set>
#include <mutex>
#include <string>
#include <unistd.h>
#include <errno.h>
std::mutex ar_mutex;

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

void ar_sender_thread(int ar_sockfd, sockaddr_in ar_peer_addr, const std::string& ar_name) {
    int ar_seq_num = 1;
    const int max_messages = 45;

    while (ar_seq_num <= max_messages) {
        std::string ar_msg = "SRC:" + ar_name + "|SEQ:" + std::to_string(ar_seq_num) + "|MSG:Hello";

        sendto(ar_sockfd, ar_msg.c_str(), ar_msg.size(), 0,
               (struct sockaddr*)&ar_peer_addr, sizeof(ar_peer_addr));

        std::cout << "[SEND][" << ar_name << "] " << ar_msg << std::endl;
        ar_seq_num++;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void ar_receiver_thread(int ar_sockfd, const std::string& ar_name) {
    sockaddr_in ar_sender_addr;
    socklen_t ar_addr_len = sizeof(ar_sender_addr);
    char ar_buffer[512];

    std::set<int> ar_received_sequences;
    int ar_expected_seq = 1;

    while (true) {
        memset(ar_buffer, 0, sizeof(ar_buffer));
        ssize_t len = recvfrom(ar_sockfd, ar_buffer, sizeof(ar_buffer) - 1, 0,
                               (struct sockaddr*)&ar_sender_addr, &ar_addr_len);
        if (len < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // Timeout: assume sender finished, exit receiver thread
                std::cout << "[INFO][" << ar_name << "] recvfrom timeout, exiting receiver thread\n";
                break;
            }
            perror("recvfrom failed");
            break;
        }

        ar_buffer[len] = '\0';
        std::string ar_msg(ar_buffer);

        // Ignore messages from self
        if (ar_msg.find("SRC:" + ar_name) != std::string::npos) {
            continue;
        }

        std::cout << "[RECV][" << ar_name << "] " << ar_msg << std::endl;

        int seq = extract_sequence(ar_msg);
        if (seq == -1) {
            std::cout << "[ERROR][" << ar_name << "] Invalid message format. Skipping...\n";
            continue;
        }

        // Lock for thread-safe access to sequences
        {
            std::lock_guard<std::mutex> lock(ar_mutex);

            if (ar_received_sequences.count(seq)) {
                std::cout << "[WARNING][" << ar_name << "] Duplicate packet detected: SEQ:" << seq << "\n";
                continue;
            }

            if (seq > ar_expected_seq) {
                std::cout << "[ERROR][" << ar_name << "] Packet loss detected! Missing SEQ:" << ar_expected_seq
                          << " to SEQ:" << (seq - 1) << "\n";
            }

            if (seq < ar_expected_seq) {
                std::cout << "[WARNING][" << ar_name << "] Out-of-order packet detected: SEQ:" << seq
                          << " (expected: " << ar_expected_seq << ")\n";
            }

            ar_received_sequences.insert(seq);
            if (seq >= ar_expected_seq) {
                ar_expected_seq = seq + 1;
            }
        if (ar_received_sequences.size() >= 30) {
            std::cout << "[INFO][" << ar_name << "] Received 30 messages. Exiting receiver...\n";
            break;
        }
        }
    }
}

int main() {
    const std::string ar_name = "udp_sender";

    int ar_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ar_sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in ar_sender_addr;
    memset(&ar_sender_addr, 0, sizeof(ar_sender_addr));
    ar_sender_addr.sin_family = AF_INET;
    ar_sender_addr.sin_port = htons(8081);  // sender listens on port 8081
    ar_sender_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(ar_sockfd, (struct sockaddr*)&ar_sender_addr, sizeof(ar_sender_addr)) < 0) {
        perror("Bind failed");
        close(ar_sockfd);
        return 1;
    }

    // Set recv timeout to 2 seconds so recvfrom doesn't block forever
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if (setsockopt(ar_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        perror("setsockopt failed");
        close(ar_sockfd);
        return 1;
    }


    // Setup peer address (receiver)
    struct hostent* he = gethostbyname("udp_receiver");
    if (he == nullptr) {
        perror("gethostbyname failed for udp_receiver");
        close(ar_sockfd);
        return 1;
    }

    sockaddr_in ar_peer_addr;
    memset(&ar_peer_addr, 0, sizeof(ar_peer_addr));
    ar_peer_addr.sin_family = AF_INET;
    ar_peer_addr.sin_port = htons(8080);
    memcpy(&ar_peer_addr.sin_addr, he->h_addr_list[0], he->h_length);

    // Give receiver time to start
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::thread sender_thread(ar_sender_thread, ar_sockfd, ar_peer_addr, ar_name);
    std::thread receiver_thread(ar_receiver_thread, ar_sockfd, ar_name);

    sender_thread.join();
    // After sending 30 messages, close socket to unblock recvfrom
    shutdown(ar_sockfd, SHUT_RDWR);
    receiver_thread.join();

    close(ar_sockfd);
    return 0;
}