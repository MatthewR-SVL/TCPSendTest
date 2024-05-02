#define WIN32_LEAN_AND_MEAN

#include "TCPIO.h"
#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512

TCPIO::TCPIO(const char* ip) {
    this->ip_addr = ip;
    this->mac_addr = "";
    this->gateway = "";
    this->subnet = "";
    this->dhcp_enabled = "";
    this->firmware_version = "";
    
    this->pnp_passthrough = NULL;
    this->pnp_state = NULL;
    this->npn_passthrough = NULL;
    this->npn_state = NULL;
    this->do3_state = NULL;
    this->do4_state = NULL;
    this->analog_passthrough = NULL;
    this->analog_state = NULL;
    this->input_polarity = NULL;
    this->trigger_mode = NULL;
    this->trigger_delay = NULL;
    this->on_time = NULL;
    this->off_time = NULL;
    this->repeat_count = NULL;

    this->get_info();
}

std::string TCPIO::send_msg_to_device(const char* port, const char* address, const char* sendbuf)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    memset(recvbuf, '\0', recvbuflen);

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return "error";
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(address, port, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return "error";
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return "error";
        }

        // Connect to server.
        std::cout << ptr->ai_addr << std::endl;
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return "error";
    }

    // Send an initial buffer
    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return "error";
    }

    printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return "error";
    }

    std::string msg = "";

    // Receive until the peer closes the connection
    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

            // check if a string is present. If present, append to msg.
            size_t recv_strlen = strlen(recvbuf);
            if (recv_strlen > 0) {
                /*std::cout << strlen(recvbuf) << std::endl;*/
                msg.append(recvbuf);
            }

            // reset recvbuf to look for more strings next loop
            memset(recvbuf, '\0', recvbuflen);
        }
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while (iResult > 0);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    // return msg
    return msg;
}

void TCPIO::get_info() {
    std::string device_info = this->send_msg_to_device(this->port, this->ip_addr, "PING");
    /*std::cout << device_info << std::endl;*/

    // Pointer to point the word returned by the strtok() function.
    char* token = NULL;
    char* next_token = NULL;
    char temp_str[DEFAULT_BUFLEN];
    memset(temp_str, '\0', DEFAULT_BUFLEN);
    strcpy_s(temp_str, DEFAULT_BUFLEN, device_info.c_str());
    /*printf(temp_str);*/
    // Here, the delimiter is a comma.
    token = strtok_s(temp_str, ",", &next_token);

    std::string info_array[21];

    int indexer = 0;
    while (token != NULL) {
        info_array[indexer] = token;
        std::cout << token << std::endl;
        token = strtok_s(NULL, ",", &next_token);
        indexer++;
    }
    
    this->mac_addr = info_array[2];
    this->subnet = info_array[3];
    this->gateway = info_array[4];
    this->dhcp_enabled = info_array[5] == "1";
    this->firmware_version = info_array[6];
    
    this->pnp_passthrough = info_array[7] == "1";
    this->pnp_state = info_array[8] == "1";
    this->npn_passthrough = info_array[9] == "1";
    this->npn_state = info_array[10] == "1";
    this->do3_state = info_array[11] == "1";
    this->do4_state = info_array[12] == "1";
    this->analog_passthrough = stoi(info_array[13]);
    this->analog_state = stoi(info_array[14]);
    this->input_polarity = info_array[15] == "1";
    this->trigger_mode = stoi(info_array[16]);
    this->trigger_delay = stoi(info_array[17]);
    this->on_time = stoi(info_array[18]);
    this->off_time = stoi(info_array[19]);
    this->repeat_count = stoi(info_array[20]);
}

void TCPIO::set_io() {
    char io_string[DEFAULT_BUFLEN];
    sprintf_s(io_string, DEFAULT_BUFLEN, "SET_IO,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
        this->pnp_state, this->npn_state, this->do3_state, this->do4_state,
        this->analog_state, this->trigger_mode, this->pnp_passthrough, this->npn_passthrough,
        this->analog_passthrough, this->input_polarity, this->trigger_delay, this->on_time, this->off_time, this->repeat_count);
    std::cout << io_string << std::endl;
    this->send_msg_to_device(this->port, this->ip_addr, io_string);
}

void TCPIO::set_io(bool pnp_state, bool npn_state, bool do3_state, bool do4_state, int analog_state) {
    this->pnp_state = pnp_state;
    this->npn_state = npn_state;
    this->do3_state = do3_state;
    this->do4_state = do4_state;
    this->analog_state = analog_state;

    this->set_io();
}

void TCPIO::set_io(long on_time, long off_time, int repeat_count) {
    this->on_time = on_time;
    this->off_time = off_time;
    this->repeat_count = repeat_count;

    this->set_io();
}