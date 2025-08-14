// sender.cpp
#include <iostream>
#include <string>
#include <bitset>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

// Generate 12-bit Hamming Code from 8-bit data
string generateHammingCode(string data) {
    string hamming(12, '0');
    int j = 0;

    // Insert data bits into the Hamming code at non-parity positions
    for (int i = 0; i < 12; ++i) {
        if (i != 0 && i != 1 && i != 3 && i != 7) {
            hamming[i] = data[j++];
        }
    }

    // Calculate parity bits
    int parity_pos[] = {0, 1, 3, 7};
    for (int p : parity_pos) {
        int parity = 0;
        for (int i = 0; i < 12; ++i) {
            if (((i + 1) & (p + 1)) && i != p) {
                parity ^= (hamming[i] - '0');
            }
        }
        hamming[p] = parity + '0';
    }

    return hamming;
}

int main() {
    char ch;
    cout << "Enter first character of your name: ";
    cin >> ch;

    bitset<8> ascii(ch);
    string data = ascii.to_string();
    string codeword = generateHammingCode(data);

    cout << "Original ASCII in binary: " << data << endl;
    cout << "Generated Hamming Code: " << codeword << endl;

    int choice;
    cout << "\nChoose an option:\n";
    cout << "1. Send data without error\n";
    cout << "2. Send data with simulated error (flip 4th bit)\n";
    cout << "Enter your choice (1 or 2): ";
    cin >> choice;

    switch (choice) {
        case 1:
            cout << "1. Sending data without introducing any error.\n";
            break;

        case 2:
            cout << "2. Introducing error: Flipping 4th bit (index 3).\n";
            codeword[3] = (codeword[3] == '0') ? '1' : '0';
            break;

        default:
            cout << "Invalid choice. Exiting.\n";
            return 1;
    }

    // Setup TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Bind sender socket explicitly to 127.0.0.40
    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = 0;  // Let OS assign ephemeral port
    local_addr.sin_addr.s_addr = inet_addr("127.0.0.40");

    if (bind(sock, (sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("Bind to local address failed");
        close(sock);
        return 1;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(12345);
    server.sin_addr.s_addr = inet_addr("127.0.0.40");

    // Connect to server
    if (connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    // Send the (possibly modified) codeword
    send(sock, codeword.c_str(), codeword.size(), 0);
    close(sock);

    cout << "Codeword sent to receiver: " << codeword << endl;
    return 0;
}
