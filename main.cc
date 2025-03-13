#include <iostream>
#include "TopasDevice.hh"


/*void printDevices(const std::vector<json>& devices){
    std::cout << "Found " << devices.size() << " Topas4 devices:" << std::endl;
    
    for (size_t i = 0; i < devices.size(); ++i){
        std::cout << "\nDevice " << (i + 1) << ":" << std::endl;
        
        // Pretty-print the entire JSON object
        std::cout << std::setw(4) << devices[i] << std::endl;
    }
}*/


int main(){
    const std::string serialNumber = "Orpheus-F-Demo-1023";

    TopasDevice* testDevice = new TopasDevice(serialNumber);
    TopasDevice::ShutterStatus status = testDevice->getShutterStatus();
    float current_wavelength = testDevice->getCurrentWavelength();

    printf("Current wavelength of device %4.1fnm\n", current_wavelength);

    delete testDevice;
    return 0;
}