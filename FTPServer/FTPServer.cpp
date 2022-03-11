#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#pragma warning(disable:4996)

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <thread>
namespace fs = std::filesystem;

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 2048
#define DEFAULT_PORT "21"

void * handle_client(SOCKET ClientSocket) {
    do {
        WSADATA wsaData;
        int iResult;
        int len;
        bool just_connected = true;

        int iSendResult;
        char recvbuf[DEFAULT_BUFLEN];
        char sendBuff[DEFAULT_BUFLEN];
        int recvbuflen = DEFAULT_BUFLEN;


        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        bool first_connection = true;
        string connect = "connect";
        for (int i = 0;i < iResult;i++) {
            if (recvbuf[i] != connect[i]) {
                first_connection = false;
                break;
            }
        }
        if (first_connection) {
            char sendBuff[DEFAULT_BUFLEN] = "";
            int i = 0, buff_len = 0;
            for (auto& p : fs::directory_iterator(fs::current_path())) {

                if (fs::is_directory(p))
                    continue;
                else {
                    string x = p.path().filename().string();
                    len = x.length();
                    uintmax_t y = fs::file_size(p);

                    for (int j = 0; j < len; j++, i++) {
                        sendBuff[i] = x[j];
                        buff_len++;
                    }

                    sendBuff[buff_len] = '\n';
                    buff_len++;
                    i++;
                    x = to_string(y);
                    for (int j = 0;j < x.length();j++, i++) {
                        sendBuff[i] = x[j];
                        buff_len++;
                    }
                    sendBuff[buff_len] = '\n';
                    buff_len++;
                    i++;
                }
            }
            iSendResult = send(ClientSocket, sendBuff, buff_len, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("1send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                //return 2;
            }
            else {
                cout << "sent folder tree" << endl;
            }
            first_connection = false;
        }
        else if (!first_connection) {
            for (auto& p : fs::directory_iterator(fs::current_path())) {

                if (fs::is_directory(p))
                    continue;
                else {
                    string x = p.path().filename().string();
                    len = x.length();
                    uintmax_t y = fs::file_size(p);
                    int z = y;
                    bool correct_file = true;
                    int i;
                    for (i = 0; i < len; i++)
                        if (recvbuf[i] != x[i])
                            correct_file = false;
                    if (correct_file) {
                        recvbuf[i] = '\0';
                        FILE* File;
                        File = fopen(recvbuf, "rb");
                        if (!File) {
                            cout << "Error while reading file";
                            break;
                        }
                        fseek(File, 0, SEEK_END);
                        fseek(File, 0, SEEK_SET);
                        do {
                            memset(sendBuff, 0, 1024);
                            fread(sendBuff, 1024, 1, File);
                            z -= 1024;
                            iSendResult = send(ClientSocket, sendBuff, 1024, 0);
                            if (iSendResult == SOCKET_ERROR) {
                                printf("2send failed with error: %d\n", WSAGetLastError());
                                closesocket(ClientSocket);
                                WSACleanup();
                                //return 2;
                            }
                            Sleep(5);
                        } while (z >= 1024);
                        if (z != 0 && z > 0) {
                            memset(sendBuff, 0, 1024);
                            fread(sendBuff, z, 1, File);
                            iSendResult = send(ClientSocket, sendBuff, 1024, 0);
                            if (iSendResult == SOCKET_ERROR) {
                                printf("3send failed with error: %d\n", WSAGetLastError());
                                closesocket(ClientSocket);
                                WSACleanup();
                                //return 2;
                            }
                        }
                        sendBuff[0] = 'd';
                        sendBuff[1] = 'o';
                        sendBuff[2] = 'n';
                        sendBuff[3] = 'e';
                        sendBuff[4] = '\0';
                        iSendResult = send(ClientSocket, sendBuff, 5, 0);
                        if (iSendResult == SOCKET_ERROR) {
                            printf("4send failed with error: %d\n", WSAGetLastError());
                            closesocket(ClientSocket);
                            WSACleanup();
                            //return 2;
                        }
                        cout << "sent to: " << ClientSocket;
                        fclose(File);
                    }
                }
            }
        }
    } while (true);
}

int __cdecl main(void)
{
    WSADATA wsaData;
    int iResult;
    int len;
    bool just_connected = true,free_thread[5];
    int number_of_connections = 5, connected = 0;
    free_thread[0] = true;free_thread[1] = true;free_thread[2] = true;free_thread[3] = true;free_thread[4] = true;
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    char sendBuff[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);


    //multiuser
    do {
        if (connected < number_of_connections) {
            iResult = listen(ListenSocket, SOMAXCONN);
            if (iResult == SOCKET_ERROR) {
                printf("listen failed with error: %d\n", WSAGetLastError());
                closesocket(ListenSocket);
                WSACleanup();
                return 1;
            }
            ClientSocket = accept(ListenSocket, NULL, NULL);
            if (ClientSocket == INVALID_SOCKET) {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(ListenSocket);
                WSACleanup();
                return 1;
            }
            if (free_thread[0]) {
                free_thread[0] = false;
                thread th0(handle_client, ClientSocket);
                th0.detach();
            }
            else if (free_thread[1]) {
                free_thread[1] = false;
                thread th1(handle_client, ClientSocket);
                th1.detach();
            }
            else if (free_thread[2]) {
                free_thread[2] = false;
                thread th2(handle_client, ClientSocket);
                th2.detach();
            }
            else if (free_thread[3]) {
                free_thread[3] = false;
                thread th3(handle_client, ClientSocket);
                th3.detach();
            }
            else if (free_thread[4]) {
                free_thread[4] = false;
                thread th4(handle_client, ClientSocket);
                th4.detach();
            }
            
        }
    } while (1);

    // No longer need server socket
    closesocket(ListenSocket);

    // cleanup
    WSACleanup();

    return 0;
}
