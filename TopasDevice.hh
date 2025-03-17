#ifndef TOPASDEVICE_HH
#define TOPASDEVICE_HH

#include <thread>
#include <chrono>

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
    void setWavelength(float wavelength, const std::string& interactionName) const;

    ShutterStatus getShutterStatus() const;
    float getCurrentWavelength() const;
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

    bool isWavelengthInRange(float wavelength, const json& item) const;
    json getInteractionFromName(const std::string& interactionName) const;
    void waitForWavelengthSetting() const;
};


#endif