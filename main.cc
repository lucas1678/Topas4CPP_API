#include <iostream>
#include "TopasLocator.hh"


void printDevices(const std::vector<json>& devices){
    std::cout << "Found " << devices.size() << " Topas4 devices:" << std::endl;
    
    for (size_t i = 0; i < devices.size(); ++i){
        std::cout << "\nDevice " << (i + 1) << ":" << std::endl;
        
        // Pretty-print the entire JSON object
        std::cout << std::setw(4) << devices[i] << std::endl;
    }
}


int main(){
    TopasLocator* testLocator = new TopasLocator();

    std::vector<json> devices = testLocator->locate();
    printDevices(devices);

    delete testLocator;
    return 0;
}