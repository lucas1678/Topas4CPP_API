#ifndef TOPASDEVICE_HH
#define TOPASDEVICE_HH

#include "TopasCommunicator.hh"

class TopasDevice{
public:
    enum class ShutterStatus{
        OPEN,
        CLOSED
    };

    static std::string ShutterStatusToString(ShutterStatus status);
    static ShutterStatus BooleanToShutterStatus(bool status);
public:
    TopasDevice(const std::string& serialNum);
    ~TopasDevice();

    void setShutterStatus(ShutterStatus status) const;
    void setWavelength(float wavelength) const;
    void setWavelength(float wavelength, const std::string& interaction) const;

    ShutterStatus getShutterStatus() const;
    float getCurrentWavelength() const;
    json getAvailableInteractions() const;
    void printDeviceInfo() const;
    void printAvailableInteractions() const;
private:
    const std::string m_serialNum;
    std::string m_baseAddress;
    bool m_initialized;
    TopasCommunicator m_http_communicator;

    //  These should be the same for all Topas devices (double check, though)
    const std::string WAVELENGTH_STATUS_ADDRESS = "/Optical/WavelengthControl/Output";
    const std::string WAVELENGTH_CONTROL_ADDRESS = "/Optical/WavelengthControl/SetWavelength";
    const std::string SHUTTER_CONTROL_ADDRESS = "/ShutterInterlock/OpenCloseShutter";
    const std::string SHUTTER_STATUS_ADDRESS = "/ShutterInterlock/IsShutterOpen";
    const std::string AVAIABLE_INTERACTIONS_ADDRESS = "/Optical/WavelengthControl/ExpandedInteractions";

    /*ShutterStatus m_shutterStatus;
    float m_wavelength;*/

    void waitForWavelengthSetting() const;
    //void updateAllSettings();  // this method should update all the member variables such as m_shutterStatus by sending appropriate HTTP requests
};


#endif