#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

const int PORT = 12000;
const int MAX_BUFFER_SIZE = 1024;
const int NUM_PINGS = 10;
const int TIMEOUT_SEC = 1;

int main() {
    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    // Create a UDP socket
    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Set timeout for recvfrom function
    timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
        std::cerr << "setsockopt for SO_RCVTIMEO failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Server address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    for (int i = 0; i < NUM_PINGS; ++i) {
        // Prepare message
        char message[MAX_BUFFER_SIZE];
        time_t now = time(nullptr);
        struct tm timeInfo;
        localtime_s(&timeInfo, &now);
        strftime(message, sizeof(message), "Ping %d-%m-%y-%H-%M-%S", &timeInfo);

        // Send message to server
        int bytesSent = sendto(clientSocket, message, strlen(message), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "sendto failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        // Receive response from server
        char buffer[MAX_BUFFER_SIZE];
        sockaddr_in fromAddr;
        int fromAddrSize = sizeof(fromAddr);
        int bytesReceived = recvfrom(clientSocket, buffer, MAX_BUFFER_SIZE, 0, (sockaddr*)&fromAddr, &fromAddrSize);
        if (bytesReceived == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAETIMEDOUT) {
                std::cout << "Request timed out" << std::endl;
            }
            else {
                std::cerr << "recvfrom failed with error: " << WSAGetLastError() << std::endl;
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }
        }
        else {
            buffer[bytesReceived] = '\0';
            std::cout << "Received: " << buffer << std::endl;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
