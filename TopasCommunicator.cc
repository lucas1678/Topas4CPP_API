#include "TopasCommunicator.hh"

// Callback function for CURL to write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
        return newLength;
    } catch(std::bad_alloc& e) {
        // Handle memory problem
        return 0;
    }
}

TopasCommunicator::TopasCommunicator(const std::string& serialNum) : m_serialNum{serialNum}, m_initialized{false} {
    //  Initialize CURL for the entire instance (globally)
    if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK){
        printf("Failed to initialize CURL!");
        return;
    }

    //  Grab Topas devices using locator, and select the one with correct serialNum
    std::vector<json> avaiableDevices = m_locator.locate();
    for(const auto& device : avaiableDevices){
        if(device["SerialNumber"] == m_serialNum){
            m_baseAddress = device["PublicApiRestUrl_Version0"];
            m_initialized = true;
            printf("Successfully initialized device at given serial number and with base address: %s\n", m_baseAddress);
            return;
        }
    }

    //  If no device found, throw a warning
    printf("Failed to find device with serial number %s\n", m_serialNum);
}

TopasCommunicator::~TopasCommunicator(){

}