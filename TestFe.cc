// LaserSystemFe.cc
//
//  MIDAS frontend for the Topas laser system at MIEL
//
// March 2025, Lucas Backes <lbackes@triumf.ca>

#include <stdio.h>
#include <signal.h> // SIGPIPE
#include <assert.h> // assert()
#include <stdlib.h> // malloc()

#include "midas.h"
#include "tmfe_rev0.h"
#include "TopasDevice.hh"

/* Callbacks for when ODB settings change */
void wavelength_callback(INT hDB, INT hkey, INT index, void *feptr);
void shutter_callback(INT hDB, INT hkey, INT index, void *feptr);

/* This class name is fe... following the midas-2020 release convention, but would now be known as an Equipment */
class feTopasDevice : public TMFePeriodicHandlerInterface
{
private:
    //const char* fSerialNumber{"P23894"};
    const char* fSerialNumber{"Orpheus-F-Demo-9388"};
public:
    TMFE* fMfe;
    TMFeEquipment* fEq;

    int fEventSize;
    char* fEventBuf;

    TopasDevice* laserEquipment{nullptr};  //  Using nullptr instead of NULL (C++11 practice. More type-safe)

public:
    feTopasDevice(TMFE* mfe, TMFeEquipment* eq) // ctor
    {
        fMfe = mfe;
        fEq = eq;
        fEventSize = 0;
        fEventBuf = nullptr;
    }

    ~feTopasDevice() // dtor
    {
        if (laserEquipment) {delete laserEquipment;}

        if (fEventBuf){
            free(fEventBuf);
            fEventBuf = NULL;
        }
    }

    void Init()
    {
        fEq->SetStatus("Initializing...", "lightgreen");

        std::cout << "\n\n\
               **********************************************************\n\
               *                                                        *\n\
               *              MIDAS MIEL Laser System INTERFACE         *\n\
               *                                                        *\n\
               *                Blah blah blah blah blah                *\n\
               *                   ~~~~~~~~~~~~~~~~~~~                  *\n\
               *                                                        *\n\
               **********************************************************\n\n";

        // Initialize event buffers for MIDAS banks
        fEventSize = (sizeof(double) + sizeof(bool)); // 1 boolean + 1 double per event (status and wavelength)
        if (fEventBuf) {free(fEventBuf);}
        fEventBuf = (char *)malloc(fEventSize);

        // Initialize ODB Settings and Variables
        // Create with default values if they don't exist, or get the current value of Settings if they do
        int delay_ms = 0;
        int num_points = 100;
        bool shutterStatus{false};
        double wavelength{-1.0};
        std::vector<std::string> avaiableInteractions;

        fEq->fOdbEqVariables->RB("OpenShutter", &shutterStatus, true);
        fEq->fOdbEqVariables->RD("Wavelength", &wavelength, true);

        // Create the TopasDevice object and connect to it (done in constructor, as of right now)
        laserEquipment = new TopasDevice();
        laserEquipment->initializeWithSerialNumber(fSerialNumber);

        if (!laserEquipment->isInitialized()){
            //  try connecting again!
            laserEquipment->initializeWithSerialNumber(fSerialNumber);
            //  only if connection fails twice, throw error
            if (!laserEquipment->isInitialized()){
                fMfe->Msg(MERROR, "Init", "Couldn't find the device. Make sure it is connected and try again.");
                fEq->SetStatus("Connection Failure", "white");
                return;
            }
        }

        // update device settings to match ODB settings
        laserEquipment->setShutterStatus(TopasDevice::BooleanToShutterStatus(shutterStatus));
        laserEquipment->setWavelength(wavelength);  // right now this picks a random interaction! CHANGE later if needed

        //  register callbacks for each setting change
        char tmpbuf[80];  //  80 bytes long temporary buffer (longer? shorter?)
        HNDLE hkey;
        sprintf(tmpbuf, "/Equipment/%s/Settings/Wavelength", fEq->fName.c_str());
        db_find_key(fMfe->fDB, 0, tmpbuf, &hkey);
        db_watch(fMfe->fDB, hkey, wavelength_callback, (void *)this);

        sprintf(tmpbuf, "/Equipment/%s/Settings/OpenShutter", fEq->fName.c_str());
        db_find_key(fMfe->fDB, 0, tmpbuf, &hkey);
        db_watch(fMfe->fDB, hkey, shutter_callback, (void *)this);

        fEq->SetStatus("Ready!", "#00FF00");
    }

    void SendEvent(bool shutterStatusToSend, double wavelengthToSend){
        fEq->ComposeEvent(fEventBuf, fEventSize);
        fEq->BkInit(fEventBuf, fEventSize);

        //  Create TWAV bank to store wavelength
        double* double_ptr = (double*) fEq->BkOpen(fEventBuf, "TWAV", TID_DOUBLE);
        *double_ptr++ = wavelengthToSend;
        fEq->BkClose(fEventBuf, double_ptr);

        //  Create TSHU bank to store shutter status
        bool* bool_ptr = (bool*) fEq->BkOpen(fEventBuf, "TSHU", TID_BOOL);
        *bool_ptr++ = shutterStatusToSend;
        fEq->BkClose(fEventBuf, bool_ptr);
    }

    /* we could make this a Polled equipment(?) */
    void HandlePeriodic()
    {
        if (!laserEquipment->isInitialized()){
            fEq->SetStatus("Connection Failure", "lightred");
            return;
        }

        bool currentShutterStatus = TopasDevice::ShutterStatusToBoolean(laserEquipment->getShutterStatus());
        double currentWavelength = (double) laserEquipment->getCurrentWavelength(); 

        // Update MIDAS with shutter status, wavelength, interactions avaiable
        SendEvent(currentShutterStatus, currentWavelength); // save data to MIDAS bank to mid.lz4 file
        fEq->fOdbEqVariables->WB("OpenShutter", currentShutterStatus);   // also save as an ODB variable
        fEq->fOdbEqVariables->WD("Wavelength", currentWavelength);
        fEq->WriteStatistics(); // update the statistics like number of events sent, etc

        // update status on MIDAS Status page
        char msg[64] = {0};
        sprintf(msg, "Wavelength: %.0f nm, Shutter Status: %i", currentWavelength, currentShutterStatus);
        fEq->SetStatus(msg, "lightgreen");
    }
};

void wavelength_callback(INT hDB, INT hkey, INT index, void *feptr)
{
    // get access to the frontend/equipment object
    feTopasDevice* fe = (feTopasDevice* )feptr;

    // get the updated value of the setting
    double wavelength;
    int size = sizeof(wavelength);
    int status = db_get_data(hDB, hkey, &wavelength, &size, TID_DOUBLE);
    if (status != DB_SUCCESS)
    {
        fe->fMfe->Msg(MERROR, "wavelength_callback", "Couldn't retrieve wavelength setting from ODB");
        return;
    }

    // send the new value to the laser system
    fe->laserEquipment->setWavelength(wavelength);

    //  check that wavelength actually changed below? (API should already do that! Can change so it return true/false)
    //  (NOT IMPLEMENTED)

    // wait for settling if applicable (LUCAS - Use this is wavelength takes a bit to "settle", though I don't think this is the case)
    /*int delay_ms = 0;
    fe->fEq->fOdbEqSettings->RI("Delay_ms", &delay_ms);
    usleep(delay_ms * 1000);*/
}

void shutter_callback(INT hDB, INT hkey, INT index, void *feptr)
{
    // get access to the frontend/equipment object
    feTopasDevice* fe = (feTopasDevice* )feptr;

    // get the updated value of the setting
    bool shutterStatusToSet;
    int size = sizeof(TopasDevice::BooleanToShutterStatus(shutterStatusToSet));  //  when sending, BOOL (1 byte) gets converted to ShutterStatus (4 bytes) 
    int status = db_get_data(hDB, hkey, &shutterStatusToSet, &size, TID_BOOL);  //  so buffer needs to be 4 bytes long, not sizeof(bool)
    if (status != DB_SUCCESS)
    {
        fe->fMfe->Msg(MERROR, "shutter_callback", "Couldn't retrieve shutter status (setting) from ODB");
        return;
    }

    // send the new value to the laser system
    fe->laserEquipment->setShutterStatus(TopasDevice::BooleanToShutterStatus(shutterStatusToSet));
    //  check that status actually changed below? (API should already do that! Though I can change it so it return true/false)
    //  (NOT IMPLEMENTED)

    /*bool success = fe->laserEquipment->setShutterStatus(TopasDevice::BooleanToShutterStatus(shutterStatusToSet));
    if (!success)
    {
        bool actual_status = TopasDevice::ShutterStatusToBoolean(fe->laserEquipment->getShutterStatus());
        fe->fMfe->Msg(MERROR, "shutter_callback", "Couldn't set wavelength on the laser system to %i, it is actually at %i", shutterStatusToSet, actual_status);
        return;
    }*/
}


int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    signal(SIGPIPE, SIG_IGN);

    std::string name = "LaserSystemFe";

    TMFE *mfe = TMFE::Instance();

    TMFeError err = mfe->Connect(name.c_str(), __FILE__);
    if (err.error)
    {
        printf("Cannot connect, bye.\n");
        return 1;
    }

    mfe->SetWatchdogSec(0); // disables timeout
    // mfe->SetWatchdogSec(15);

    TMFeCommon *common = new TMFeCommon();
    common->EventID = 3;
    common->LogHistory = 1;
    common->Buffer = "SYSTEM";

    TMFeEquipment *eq = new TMFeEquipment(mfe, "LaserEquipment", common);
    eq->Init();
    eq->SetStatus("Starting...", "white");
    eq->ZeroStatistics();
    eq->WriteStatistics();

    mfe->RegisterEquipment(eq);

    feTopasDevice *myfe = new feTopasDevice(mfe, eq);
    myfe->Init();
    mfe->RegisterPeriodicHandler(eq, myfe);

    while (!mfe->fShutdownRequested)
    {
        mfe->PollMidas(10);
    }

    // do cleanup tasks. It seems I get a warning when I try to delete the dynamically allocated memory here (in particular, deleting myfe), but we really do want to make sure picometer gets deleted because the destructor does safety cleanup
    if (myfe->laserEquipment)
    {
        delete myfe->laserEquipment;
    }

    eq->SetStatus("Stopped", "white");
    mfe->Disconnect();

    return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
