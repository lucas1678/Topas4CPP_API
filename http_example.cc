#include "TopasCommunicator.hh"

void PrintHttpResponse(json response){
    std::cout << "The received HTTP JSON response is: " << response << std::endl;
}

void PrettyPrintHttpResponse(json response){
    std::cout << "The received HTTP JSON response is: \n";
    for(const auto& item : response){
        std::cout << item << std::endl;
    }
}

void printDevices(const std::vector<json>& devices){
    std::cout << "Found " << devices.size() << " Topas4 devices:" << std::endl;
    
    for (size_t i = 0; i < devices.size(); ++i){
        std::cout << "\nDevice " << (i + 1) << ":" << std::endl;
        
        // Pretty-print the entire JSON object
        std::cout << std::setw(4) << devices[i] << std::endl;
    }
}

int main(){
    const std::string WAVELENGTH_STATUS_ADDRESS = "/Optical/WavelengthControl/Output";
    const std::string WAVELENGTH_CONTROL_ADDRESS = "/Optical/WavelengthControl/SetWavelength";
    const std::string SHUTTER_CONTROL_ADDRESS = "/ShutterInterlock/OpenCloseShutter";
    const std::string SHUTTER_STATUS_ADDRESS = "/ShutterInterlock/IsShutterOpen";
    const std::string AVAIABLE_INTERACTIONS_ADDRESS = "/Optical/WavelengthControl/ExpandedInteractions";

    TopasLocator* topasLocator = new TopasLocator();
    std::vector<json> devices = topasLocator->locate();
    printDevices(devices);
    delete topasLocator;


    TopasCommunicator* httpCommunicator = new TopasCommunicator();
    httpCommunicator->initializeWithBaseAddress("http://142.90.111.190:8004/P23894/v0/PublicAPI");
    //httpCommunicator->initializeWithBaseAddress("http://142.90.108.148:8004/Orpheus-F-Demo-1023/v0/PublicAPI");
    //httpCommunicator->initializeWithSerialNumber("Orpheus-F-Demo-1023");
    json res;


    res = httpCommunicator->get(SHUTTER_STATUS_ADDRESS);
    PrintHttpResponse(res);

    res = httpCommunicator->get("/Authentication/UsersWithAccessRights");
    PrettyPrintHttpResponse(res);

    res = httpCommunicator->post("/Authentication/StartAuthenticationByInterlock", json());
    std::cout<<"Sent request to authenticate, with response: "<<std::endl;
    PrettyPrintHttpResponse(res);

    res = httpCommunicator->get("/Authentication/AuthenticationStatus");
    std::cout << "Authentication status HTTP response is: " << std::endl;
    PrintHttpResponse(res);

    delete httpCommunicator;
    return 0;
}
