//
// Created by Víctor Jiménez Rugama on 1/31/22.
//
#include "utils.h"
#include <string>

#if __APPLE__
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#elif _WIN32
#include <winsock2.h>
#endif


#ifdef _WIN32
std::string getIP(){
    std::string IPV4 = "ERROR: NO INTERNET ADAPTER FOUND";
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
        return 0;
    char local[255] = { 0 };
    gethostname(local, sizeof(local));
    auto hostInfo = gethostbyname(&local[0]);
    if(hostInfo)
    {
        for( int i=0 ; hostInfo->h_addr_list[i] ; ++i )
        {
            const in_addr* address = (in_addr*)hostInfo->h_addr_list[i] ;
            IPV4 = inet_ntoa( *address );
        }
    }
    WSACleanup();
    return IPV4;
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