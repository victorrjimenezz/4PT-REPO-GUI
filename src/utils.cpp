//
// Created by Víctor Jiménez Rugama on 1/31/22.
//
#include "utils.h"
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <ws2def.h>
#pragma comment(lib, "Ws2_32.lib")
#include <windows.h>
#include <io.h>
#elif __APPLE__
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


#ifdef _WIN32
std::string getIP(){
    WSADATA wsa_Data;
    int wsa_ReturnCode = WSAStartup(0x101,&wsa_Data);
    char szHostName[255];
    gethostname(szHostName, 255);
    struct hostent *host_entry;
    host_entry=gethostbyname(szHostName);
    char * szLocalIP;
    szLocalIP = inet_ntoa (*(struct in_addr *)*host_entry->h_addr_list);
    WSACleanup();
    return {szLocalIP};
}
#elif __APPLE__

std::string getIP(){
    char host[256];
    char *IP;
    struct hostent *host_entry;
    int hostname;
    hostname = gethostname(host, sizeof(host)); //find the host name
    if (hostname == -1) {
        perror("gethostname");
        exit(1);
    }

    host_entry = gethostbyname(host); //find host information
    if (host_entry == nullptr){
        perror("gethostbyname");
        exit(1);
    }
    IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); //Convert into IP string
    if (IP == nullptr) {
        perror("inet_ntoa");
        exit(1);
    }

    return {IP};
}
#endif