#include "TopasDevice.hh"

TopasDevice::TopasDevice(const std::string& serialNum) : 
    m_serialNum{serialNum}, 
    m_initialized{false}, 
    m_http_communicator(serialNum)
    //m_shutterStatus{ShutterStatus::CLOSED} 
{
    m_baseAddress = m_http_communicator.baseAddress();
    m_initialized = m_http_communicator.isInitialized();

    // Use communicator to get shutter status, wavelength
    bool isShutterOpen = m_http_communicator.get("/ShutterInterlock/IsShutterOpen").get<bool>();
    m_shutterStatus = isShutterOpen ? ShutterStatus::OPEN : ShutterStatus::CLOSED;

    json wavelengthControlResponse = m_http_communicator.get("/Optical/WavelengthControl/Output");
    m_wavelength = wavelengthControlResponse["Wavelength"].get<float>();


}

TopasDevice::~TopasDevice(){

}

float TopasDevice::getCurrentWavelength() const {
    return m_wavelength;
}

TopasDevice::ShutterStatus TopasDevice::getShutterStatus() const {
    return m_shutterStatus;
}

void TopasDevice::getDeviceInfo() const {
    
}

void TopasDevice::setWavelength(float wavelength){

}

void TopasDevice::setShutterStatus(ShutterStatus status){

}