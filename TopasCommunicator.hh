#ifndef TOPASCOMMUNICATOR_HH
#define TOPASCOMMUNICATOR_HH

#include <string>
#include <curl/curl.h>
#include "TopasLocator.hh"

class TopasCommunicator{
public:
    TopasCommunicator(const std::string& serialNum);
    ~TopasCommunicator();

private:
    const std::string m_serialNum;
    TopasLocator m_locator;
    bool m_initialized;
    std::string m_baseAddress;
};


#endif