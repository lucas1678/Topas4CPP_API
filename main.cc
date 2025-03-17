#include "TopasDevice.hh"


/*void printDevices(const std::vector<json>& devices){
    std::cout << "Found " << devices.size() << " Topas4 devices:" << std::endl;
    
    for (size_t i = 0; i < devices.size(); ++i){
        std::cout << "\nDevice " << (i + 1) << ":" << std::endl;
        
        // Pretty-print the entire JSON object
        std::cout << std::setw(4) << devices[i] << std::endl;
    }
}*/


int promptUser(){
    int userChoice{0};
    std::cout << "Please enter a number: \n\n" << "1. Get shutter status\t2. Get current wavelength\t3. Get avaiable interactions\n";
    std::cout << "4. Set shutter status to OPEN\t5. Set shutter status to CLOSED\n6. Set new wavelength\t";
    std::cout << "7. Set new wavelength with interaction\t8. Exit" << std::endl;
    std::cin >> userChoice;

    return userChoice;
}

int main(){
    const std::string serialNumber = "Orpheus-F-Demo-1023";
    TopasDevice* device1 = new TopasDevice(serialNumber);

    while(true){
        int userChoice = promptUser();

        std::string currentStatus{"EMPTY STATUS"};
        std::string interactionToUse{"NONE"};
        float currentWavelength{-1};
        float wavelengthToSet{-1};
        bool exitMenuLoop{false};

        switch(userChoice){
            case(1):
                currentStatus =  TopasDevice::ShutterStatusToString(device1->getShutterStatus());
                std::cout << "Current shutter status is: " << currentStatus << std::endl;
                break;
            case(2):
                currentWavelength = device1->getCurrentWavelength();
                std::cout << "Current wavelength is " << currentWavelength << "nm" << std::endl;
                break;
            case(3):
                device1->printAvailableInteractions();
                break;
            case(4):
                device1->setShutterStatus(TopasDevice::ShutterStatus::OPEN);
                break;
            case(5):
                device1->setShutterStatus(TopasDevice::ShutterStatus::CLOSED);
                break;
            case(6):
                std::cout << "Please enter the new wavelength (nm) you'd like to set: ";
                std::cin >> wavelengthToSet;
                device1->setWavelength(wavelengthToSet);
                break;
            case(7):
                std::cout << "Please enter the new wavelength (nm) you'd like to set: ";
                std::cin >> wavelengthToSet;
                std::cout << "Please enter the interaction you'd like to use: ";
                std::getline(std::cin, interactionToUse);  //calling this twice to consume trailing newline character. Probably not the prettiest fix though
                std::getline(std::cin, interactionToUse);
                device1->setWavelength(wavelengthToSet, interactionToUse);
                break;
            case(8): 
                std::cout << "Exiting!" << std::endl;
                exitMenuLoop = true; 
                break;
            default: 
                std::cout << "Invalid option. Try again!" << std::endl; 
                break;
        }
        if(exitMenuLoop==true) {break;}
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    delete device1;
    return 0;
}