#include "TopasLocator.hh"

TopasLocator::TopasLocator(){
    #ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2,2);
    err = WSAStartup(wVersionRequested, &wsaData);

    if(err!=0){
        // Tell user we could not find usable Winsock DLL
        printf("WSAStartup failed with error: %d\n", err);
    }
    #endif
}

TopasLocator::~TopasLocator(){
    #ifdef _WIN32
    WSACleanup();
    #endif
}

std::vector<json> TopasLocator::locate(){
    //  Create a UDP socket
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == INVALID_SOCKET){
        #ifdef _WIN32
            printf("Socket creation failed with error %ld. Returning empty vector\n", WSAGetLastError());
        #else
            printf("Socket creation failed with error %d: %s. Returning empty vector\n", errno, strerror(errno));
        #endif
        return {};
    }

    //  Define multicast address
    struct sockaddr_in multicastAddr;
    memset(&multicastAddr, 0, sizeof(multicastAddr)); //basically populates this address with a bunch of 0s first. To prevent random behaviour.
    multicastAddr.sin_family = AF_INET;
    //multicastAddr.sin_addr.s_addr = inet_addr("239.0.0.181");
    multicastAddr.sin_addr.s_addr = inet_addr("239.0.0.181");
    multicastAddr.sin_port = htons(7415);

    //  Define localhost address
    struct sockaddr_in localhostAddr;
    memset(&localhostAddr, 0, sizeof(localhostAddr));
    localhostAddr.sin_family = AF_INET;
    localhostAddr.sin_addr.s_addr = inet_addr("127.255.255.255"); // double check this with API documentation
    localhostAddr.sin_port = htons(7415);

    //  Define message to send. This will locate Topas4 devices.
    std::string message = "Topas4?";

    //  Send message to multicast address
    int send_result = 0;
    send_result = sendto(sock, message.c_str(), message.length(), 0, (struct sockaddr*)&multicastAddr, sizeof(multicastAddr));
    if(send_result == SOCKET_ERROR){
        #ifdef _WIN32
            printf("Sending to multicast address failed with error %ld\n", WSAGetLastError());
        #else
            printf("Sending to multicast address failed with error %d: %s\n", errno, strerror(errno));
        #endif
    }

    //  Send message to localhost address
    send_result = sendto(sock, message.c_str(), message.length(), 0, (struct sockaddr*)&localhostAddr, sizeof(localhostAddr));
    if(send_result == SOCKET_ERROR){
        #ifdef _WIN32
            printf("Sending to localhost address failed with error %ld\n", WSAGetLastError());
        #else
            printf("Sending to localhost address failed with error %d: %s\n", errno, strerror(errno));
        #endif
    }

    //  Configure socket settings such that we spend at most 1 second listening for a response
    #ifdef _WIN32
        DWORD timeout = 1000;  // DWORD is basically Windows specific unsigned int!
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));  // can add a check to see if this fails!
    #else
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    #endif

    //  Set up variables to receive a response
    std::vector<json> devices;
    const size_t buffer_size = 4096;
    char buffer[buffer_size]; //  buffer that is 4096 bytes long (sizeof(char) -> 1 byte)
    
    struct sockaddr_in senderAddr;
    socklen_t senderAddrSize = sizeof(senderAddr);

    //  Loop to listen to a response!
    while(true){
        int bytesReceived = recvfrom(sock, buffer, buffer_size, 0, (struct sockaddr*)&senderAddr, &senderAddrSize);
        if(bytesReceived == SOCKET_ERROR){
            #ifdef _WIN32
                if(WSAGetLastError() == WSAETIMEDOUT) {break;}
                printf("Error receiving data: %ld\n", WSAGetLastError());
            #else
                if(errno == EAGAIN || errno == EWOULDBLOCK) {break;}
                printf("Error receiving data. Errno %d: %s\n", errno, strerror(errno));
            #endif
        }

        // Set message endpoint to the null terminator
        buffer[bytesReceived] = '\0';

        // Parse the message on the buffer using JSON parse. Check if message is valid.
        try{
            json description = json::parse(buffer);
            if(description.contains("Identifier") && description["Identifier"]=="Topas4"){
                devices.push_back(description);
            }
        } catch (const json::parse_error& err){
            std::cerr << "(JSON) Bad data received by locator: " << err.what() << std::endl;
        }
    }

    closesocket(sock);

    //  Remove duplicate devices
    std::vector<json> uniqueDevices;
    std::set<std::string> seenGUIDS;

    for (const auto& device : devices){
        if(device.contains("SenderGUID")){
            std::string guid = device["SenderGUID"];
            if(seenGUIDS.find(guid) == seenGUIDS.end()){
                seenGUIDS.insert(guid);
                uniqueDevices.push_back(device);
            }
        }
    }

    return uniqueDevices;
}

