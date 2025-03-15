#include "TopasDevice.hh"

//  Maybe a better idea would be to leave any logic out of the constructor and make a init() method instead
//  This way a device object can be created anywhere, and it leaves the choice of when to initialize it to the user.
//  Can implement/change later...
TopasDevice::TopasDevice(const std::string& serialNum) : 
    m_serialNum{serialNum}, 
    m_initialized{false}, 
    m_http_communicator(serialNum)
    //m_shutterStatus{ShutterStatus::CLOSED} 
{
    m_baseAddress = m_http_communicator.baseAddress();
    m_initialized = m_http_communicator.isInitialized();


    //  I am choosing to not have member variables to represent device status
    //  Instead, the status of various things (shutter, wavelength etc) is only avaiable through
    //  Getter methods which send HTTP requests everytime. This way information is always up-to-date
    //  But at the cost of sending extra HTTP requests. If latency becomes an issue, can switch back to using member vars.

    /*
    // Use communicator to get shutter status, wavelength
    bool isShutterOpen = m_http_communicator.get("/ShutterInterlock/IsShutterOpen").get<bool>();
    m_shutterStatus = isShutterOpen ? ShutterStatus::OPEN : ShutterStatus::CLOSED;

    json wavelengthControlResponse = m_http_communicator.get("/Optical/WavelengthControl/Output");
    m_wavelength = wavelengthControlResponse["Wavelength"].get<float>();
    */
}

TopasDevice::~TopasDevice(){

}

std::string TopasDevice::ShutterStatusToString(ShutterStatus status){
    switch(status){
        case(ShutterStatus::OPEN): return "OPEN";
        case(ShutterStatus::CLOSED): return "CLOSED";
        default: return "UNKNOWN";
    }
}

TopasDevice::ShutterStatus TopasDevice::BooleanToShutterStatus(bool status){
    return (status) ? ShutterStatus::OPEN : ShutterStatus::CLOSED;
}

float TopasDevice::getCurrentWavelength() const {
    json data = m_http_communicator.get(WAVELENGTH_STATUS_ADDRESS);
    return data["Wavelength"].get<float>();
}

TopasDevice::ShutterStatus TopasDevice::getShutterStatus() const {
    bool isShutterOpen = m_http_communicator.get(SHUTTER_STATUS_ADDRESS).get<bool>();
    return BooleanToShutterStatus(isShutterOpen);
}

void TopasDevice::printDeviceInfo() const {
    //  Display base address, serial number, shutter status, current set wavelength, etc.
}

void TopasDevice::printAvailableInteractions() const {
    json interactions = m_http_communicator.get(AVAIABLE_INTERACTIONS_ADDRESS);
    std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    std::cout << "\nThe following interactions are avaiable:\n";
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    for(const auto& item : interactions){
        std::cout << std::setprecision(4);
        std::cout << "Interaction Type: " << item["Type"] << "\n";
        std::cout << "Valid for range: " << (float)item["OutputRange"]["From"] << "nm - " << (float)item["OutputRange"]["To"] << "nm\n\n";
    }
    //std::cout << data << std::endl;
}

json TopasDevice::getAvailableInteractions() const {
    json interactions = m_http_communicator.get(AVAIABLE_INTERACTIONS_ADDRESS);
    return interactions;
}

void TopasDevice::setWavelength(float wavelength) const {

}

void TopasDevice::setWavelength(float wavelength, const std::string& interaction) const {

}

void TopasDevice::setShutterStatus(ShutterStatus status) const {
    json response;
    switch(status){
        case(ShutterStatus::OPEN):
            std::cout << "Requesting to open shutter... " << std::endl;
            response = m_http_communicator.put(SHUTTER_CONTROL_ADDRESS, true);
            //  CHECK IF CHANGE WAS SUCCESSFUL WITH ANOTHER HTTP REQUEST
            break;
        case(ShutterStatus::CLOSED):
            std::cout << "Requesting to close shutter... " << std::endl;
            response = m_http_communicator.put(SHUTTER_CONTROL_ADDRESS, false);
            //  CHECK IF CHANGE WAS SUCCESSFUL WITH ANOTHER HTTP REQUEST
            break;
        default:
            std::cerr << "setShutterStatus received unknown ShutterStatus type. Please try again" << std::endl;
            break;
    }
}