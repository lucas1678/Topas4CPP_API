#ifndef TOPASDEVICE_HH
#define TOPASDEVICE_HH

#include "TopasCommunicator.hh"

class TopasDevice{
public:
    enum class ShutterStatus{
        OPEN,
        CLOSED
    };
public:
    TopasDevice(const std::string& serialNum);
    ~TopasDevice();

    ShutterStatus getShutterStatus() const;
    void setShutterStatus(ShutterStatus status);
    float getCurrentWavelength() const;
    void setWavelength(float wavelength);

    void getDeviceInfo() const;
private:
    const std::string m_serialNum;
    std::string m_baseAddress;
    bool m_initialized;
    TopasCommunicator m_http_communicator;

    ShutterStatus m_shutterStatus;
    float m_wavelength;

    void waitForWavelengthSetting() const;
};


#endif