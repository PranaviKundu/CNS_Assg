// receiver.cpp

#include <iostream>
#include <string>
#include <bitset>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

// Detect error position in the received Hamming codeword
int detectError(const string& codeword) {
    int error_pos = 0;
    int parity_pos[] = {0, 1, 3, 7};

    for (int i = 0; i < 4; ++i) {
        int p = parity_pos[i];
        int parity = 0;

        for (int j = 0; j < 12; ++j) {
            if (((j + 1) & (p + 1)) && j != p) {
                parity ^= (codeword[j] - '0');
            }
        }

        if (parity != (codeword[p] - '0')) {
            error_pos += (p + 1);
        }
    }

    return error_pos;
}

// Correct a single-bit error at the given position
string correctError(string codeword, int error_pos) {
    if (error_pos > 0 && error_pos <= 12) {
        codeword[error_pos - 1] = (codeword[error_pos - 1] == '0') ? '1' : '0';
    }
    return codeword;
}

// Extract original 8-bit data from 12-bit codeword
string extractDataBits(const string& codeword) {
    string data;
    for (int i = 0; i < 12; ++i) {
        if (i != 0 && i != 1 && i != 3 && i != 7) {
            data += codeword[i];
        }
    }
    return data;
}

int main() {
    char buffer[13] = {0}; // 12 bits + null terminator

    // Setup TCP socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(12345);
    address.sin_addr.s_addr = inet_addr("127.0.0.40");

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    cout << "Receiver ready, waiting for connection...\n";

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int new_socket = accept(server_fd, (sockaddr*)&client_addr, &client_len);
    if (new_socket < 0) {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    cout << "Connected from: " << client_ip << endl;

    // Receive codeword
    ssize_t bytesRead = read(new_socket, buffer, 12);
    if (bytesRead < 0) {
        perror("Read failed");
        close(new_socket);
        close(server_fd);
        return 1;
    }

    close(new_socket);
    close(server_fd);

    string codeword(buffer, 12);
    cout << "\nReceived Codeword: " << codeword << endl;

    // Detect and handle error using switch-case
    int error_pos = detectError(codeword);

    switch (error_pos) {
        case 0:
            cout << "1. Data received without Error.\n";
            break;
        default:
            cout << "2. Data received with error at position: " << error_pos << endl;
            codeword = correctError(codeword, error_pos);
            cout << "   Corrected Codeword: " << codeword << endl;
            break;
    }

    // Decode and display character
    string data = extractDataBits(codeword);
    bitset<8> bits(data);
    char decoded = static_cast<char>(bits.to_ulong());

    cout << "Decoded Character: " << decoded << endl;

    return 0;
}
