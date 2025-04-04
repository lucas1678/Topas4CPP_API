#ifndef TOPASCOMMUNICATOR_HH
#define TOPASCOMMUNICATOR_HH

#include <string>
#include <curl/curl.h>
#include "TopasLocator.hh"

class TopasCommunicator{
public:
    TopasCommunicator(const std::string& serialNum);
    TopasCommunicator();
    ~TopasCommunicator();

    bool initializeWithSerialNumber(const std::string& serialNum);
    bool initializeWithBaseAddress(const std::string& baseAddress);

    json get(const std::string& url) const;
    json put(const std::string& url, const json& data) const;
    json post(const std::string& url, const json& data) const;

    bool isInitialized() const;
    std::string baseAddress() const;
    void setBaseAddress(const std::string& baseAddressToSet);

private:
    std::string m_serialNum;
    TopasLocator m_locator;
    bool m_initialized;
    std::string m_baseAddress;
};


#endif