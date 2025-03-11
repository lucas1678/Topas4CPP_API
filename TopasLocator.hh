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
#endif


using json = nlohmann::json;

class TopasLocator {
public:
    TopasLocator();
    ~TopasLocator();
    
    std::vector<json> locate();
};


#endif