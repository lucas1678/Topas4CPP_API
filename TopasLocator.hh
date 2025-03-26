#ifndef TOPASLOCATOR_HH
#define TOPASLOCATOR_HH

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <cstring>
#include <sys/types.h>
#include <nlohmann/json.hpp> // Using nlohmann/json library for JSON parsing

#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #pragma comment(lib, "WS2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    typedef int SOCKET;
    #define SOCKET_ERROR -1
    #define INVALID_SOCKET -1
    #define closesocket close
#endif


using json = nlohmann::json;

class TopasLocator {
public:
    TopasLocator();
    ~TopasLocator();
    
    std::vector<json> locate();
};


#endif