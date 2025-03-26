#include "TopasDevice.hh"

//  Maybe a better idea would be to leave any logic out of the constructor and make a init() method instead
//  This way a device object can be created anywhere, and it leaves the choice of when to initialize it to the user.
//  Can implement/change later...
TopasDevice::TopasDevice() : 
    m_serialNum{""}, 
    m_initialized{false}, 
    m_http_communicator()
    //m_shutterStatus{ShutterStatus::CLOSED} 
{
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

void TopasDevice::initializeWithSerialNumber(const std::string& serialNum){
    m_serialNum = serialNum;
    m_initialized = m_http_communicator.initializeWithSerialNumber(serialNum);
    if(!m_initialized){
        std::cerr << "[ERROR] Failed to initialize http_communicator to serial number: " << serialNum << std::endl;
        return;
    }
    std::cout << "Successfully initialized device with serial number: " << serialNum << std::endl;
}

void TopasDevice::initializeWithBaseAddress(const std::string& baseAddress){
    m_initialized = m_http_communicator.initializeWithBaseAddress(baseAddress);
    if(!m_initialized){
        std::cerr << "[ERROR] Failed to initialize http_communicator to base address: " << baseAddress << std::endl;
        return;
    }
    std::cout << "Successfully initialized device with base address: " << baseAddress << std::endl;
}

bool TopasDevice::isInitialized() const {
    return m_initialized;
}

std::string TopasDevice::ShutterStatusToString(ShutterStatus status){
    switch(status){
        case(ShutterStatus::OPEN): return "OPEN";
        case(ShutterStatus::CLOSED): return "CLOSED";
        default: return "UNKNOWN";
    }
}

bool TopasDevice::ShutterStatusToBoolean(ShutterStatus status){
    switch(status){
        case(ShutterStatus::OPEN): return true;
        case(ShutterStatus::CLOSED): return false;
        default:
            std::cerr << "[WARNING] Unkown shutter status variable sent into TopasDevice::ShutterStatusToBoolean. ";
            std::cerr << "Returning FALSE by default!" << std::endl; 
            return false;
    }
}

TopasDevice::ShutterStatus TopasDevice::BooleanToShutterStatus(bool status){
    return (status) ? ShutterStatus::OPEN : ShutterStatus::CLOSED;
}

bool TopasDevice::isWavelengthInRange(float wavelength, const json& interaction) const {
    float lowerBound = (float) interaction["OutputRange"]["From"];
    float upperBound = (float) interaction["OutputRange"]["To"];
    if((wavelength >= lowerBound) && (wavelength <= upperBound)) {return true;}
    else {return false;}
}

json TopasDevice::getInteractionFromName(const std::string& interactionName) const {
    json interactions = m_http_communicator.get(AVAIABLE_INTERACTIONS_ADDRESS);
    for(const auto& item : interactions){
        if(item["Type"] == interactionName) {return item;}
    }
    std::cerr << "[WARNING] Could not find interaction with name " << interactionName << ". Returning EMPTY JSON string..." << std::endl;
    return json();

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

//  Sets the wavelength using the first interaction which is in the proper wavelength range
void TopasDevice::setWavelength(float wavelengthToSet) const {
    json interactions = m_http_communicator.get(AVAIABLE_INTERACTIONS_ADDRESS);
    for(const auto& item : interactions){
        if(isWavelengthInRange(wavelengthToSet, item)==true){
            //  set wavelength using the selected interaction
            std::cout << "Setting wavelength of " << wavelengthToSet << " using interaction: " << item["Type"] << std::endl;
            json data = {
                {"Interaction", item["Type"]},
                {"Wavelength", wavelengthToSet}
            };
            json response = m_http_communicator.put(WAVELENGTH_CONTROL_ADDRESS, data);
            this->waitForWavelengthSetting();
            //  wait one second and check if changes went through
            //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if(this->getCurrentWavelength()!=wavelengthToSet){
                std::cerr << "[WARNING] HTTP request sent, but value has failed to update after 1 second" << std::endl;
                return;
            }

            std::cout << " Success!" << std::endl;
            return;
        }
    }

    std::cout << "[ERROR] No interaction avaiable to set wavelength of " << wavelengthToSet << "nm" << std::endl;

}

void TopasDevice::setWavelength(float wavelengthToSet, const std::string& interactionName) const {
    //  get the appropriate JSON data based on interaction name
    json interaction = getInteractionFromName(interactionName);
    if(interaction.empty()){
        std::cerr << "[ERROR] Failed to set wavelength due to unrecognized interaction name" << std::endl;
        return;
    }
    
    //  check if parameters are valid
    if(isWavelengthInRange(wavelengthToSet, interaction)==false){
        std::cerr << "[ERROR] Out of range error. Cannot set wavelength of " << wavelengthToSet << "nm" << "using interaction: ";
        std::cerr << interaction["Type"] << std::endl;
        return;
    }

    //  pack data to send in request into a JSON format
    json data = {
        {"Interaction", interaction["Type"]},
        {"Wavelength", wavelengthToSet}
    };

    //  send HTTP request
    std::cout << "Setting wavelength of " << wavelengthToSet << " using interaction: " << interaction["Type"] << std::endl;
    json response = m_http_communicator.put(WAVELENGTH_CONTROL_ADDRESS, data);
    this->waitForWavelengthSetting();

    //  wait one second and check if changes went through (maybe remove this)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if(this->getCurrentWavelength()!=wavelengthToSet){
        std::cerr << "[WARNING] HTTP request sent, but value has failed to update after 1 second" << std::endl;
        return;
    }
    
    std::cout << " Success!" << std::endl;
    return;
}

void TopasDevice::waitForWavelengthSetting() const {
    while(true){
        json statusData = m_http_communicator.get(WAVELENGTH_STATUS_ADDRESS);
        float percentCompletion = (float) statusData["WavelengthSettingCompletionPart"] * 100.0;
        
        printf("\rWavelength change in progress. %.1f %% complete!", percentCompletion);

        if(statusData["IsWavelengthSettingInProgress"] == false || statusData["IsWaitingForUserAction"] == true){
            break;
        }

        //  Code below only executes after wavelength setting complete or user action requested!
        //  Query for the status data again:
        statusData = m_http_communicator.get(WAVELENGTH_STATUS_ADDRESS);
        if(statusData["IsWaitingForUserAction"] == true){
            std::cout << "\nUser actions required: \n";
            for(const auto& msg : statusData["Messages"]){
                //  print out each message to the user
                std::cout << msg["Text"].get<std::string>() << ' ';
                if(msg["Image"].is_null()){
                    std::cout << std::endl;
                } 
                else{
                    std::cout << ", image name: " << msg["Image"] << std::endl;
                }
            }
            //  wait for user input (hitting Enter key)...
            std::cout << "\nHit Enter to continue after actions have been performed. " << std::endl;
            std::cout << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  //  skip bad input
            std::cin.get();

            //  tell the device that required actions have been performed.  
            //  if shutter was open before setting wavelength it will be opened again
            m_http_communicator.put("/Optical/WavelengthControl/FinishWavelengthSettingAfterUserActions", {{"RestoreShutter", true}});
        }
    }
    //std::cout << "Done setting the wavelength!" << std::endl;
}

void TopasDevice::setShutterStatus(ShutterStatus statusToSet) const {
    json response;
    switch(statusToSet){
        case(ShutterStatus::OPEN):
            std::cout << "Requesting to open shutter... " << std::endl;
            response = m_http_communicator.put(SHUTTER_CONTROL_ADDRESS, true);
            std::cout << "Response from request to open shutter is: " << response << std::endl;
            break;
        case(ShutterStatus::CLOSED):
            std::cout << "Requesting to close shutter... " << std::endl;
            response = m_http_communicator.put(SHUTTER_CONTROL_ADDRESS, false);
            break;
        default:
            std::cerr << "setShutterStatus received unknown ShutterStatus type. Please try again" << std::endl;
            return;
    }

    //  wait one second and check if changes went through
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if(this->getShutterStatus()!=statusToSet){
        std::cerr << "[WARNING] HTTP request sent, but value has failed to update after 1 second" << std::endl;
        return;
    }

    std::cout << "Success!" << std::endl;
    return;
}