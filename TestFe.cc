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
#include "TopasDevice.hh"

/* Callbacks for when ODB settings change */
// If wanting to implement, take a look at Maia's KeySight Frontend as an example

/* This class name is fe... following the midas-2020 release convention, but would now be known as an Equipment */
class feTopasDevice : public TMFePeriodicHandlerInterface
{
private:
    const char[] fSerialNumber{"P23894"};
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
        fEventSize{0};
        fEventBuf{nullptr};
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

        fEq->fOdbEqVariables->RB("Shutter Status", &shutterStatus, true);
        fEq->fOdbEqVariables->RD("Wavelength", &wavelength, true);
        //  How to read collection of strings (from MIDAS) to put into avaiable interactions? Use a for loop?
        //fEq->fOdbEqSettings->RI("Readings_Per_Buffer", &num_points, true);
        //fEq->fOdbEqSettings->RI("Delay_ms", &delay_ms, true);

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
        laserEquipment->setShutterStatus(shutterStatus);
        laserEquipment->setWavelength(wavelength);  // right now this picks a random interaction! CHANGE later if needed

        // (IF USING CALLBACKS LATER) Register callbacks for each setting change here
        // Delay_ms doesn't need a callback

        fEq->SetStatus("Ready!", "#00FF00");
    }

    /* Saves a current readout event.
     * This program creates three banks in each event: KBCS, KBCR, and KBCT.
     *
     * The bank KACS (Keysight b2985A Current Summary) will contain four double-precision values:
     *    1. The mean of 100 current readings, in Amps (assume KB6517_POINTS_PER_BUF = 100)
     *    2. The standard deviation of the 100 current readings
     *    3. The voltage source's commanded output, in Volts (i.e. commanded output = voltage level && voltage on?)
     *    4. The integration period used for the measurements, in Number of Power Line Cycles
     *
     * The bank KACR (Keysight b2985A Current Raw) will contain all 100 current readings, in Amps. These are stored as 32-bit floats
     * The bank KATR (Keysight b2985A Time Raw) will contain the corresponding timestamps for all 100 current readings, in seconds since the first reading
     *
     * @param dvalue1 the mean current reading
     * @param dvalue2 the standard deviation
     * @param dvalue3 the commanded voltage output
     * @param dvalue4 the integration period used
     *
     * @param raw_data an array of bytes, IEEE-754 single-precision (32 bit), alternating Current, Time, Current, Time as output by the Keysight 6517B
     * @param size the number of data points (Current, Time) in the buffer. i.e. one data point uses 8 bytes
     */
    void SendEvent(bool shutterStatusToSend, double wavelengthToSend){
        fEq->ComposeEvent(fEventBuf, fEventSize);
        fEq->BkInit(fEventBuf, fEventSize);

        //  Create TWAV bank to store wavelength
        double* dptr = (double*) fEq->BkOpen(fEventBuf, "TWAV", TID_DOUBLE);
        *dptr++ = wavelengthToSend;
        fEq->BkClose(fEventBuf, dptr);

        //  Create TSHU bank to store shutter status
        bool* bptr = (bool*) fEq->BkOpen(fEventBuf, "TSHU", TID_BOOL);
        *bptr++ = shutterStatusToSend;
        fEq->BkClose(fEventBuf, fEventSize);
    }

    /* we could make this a Polled equipment, just have to do some fiddling to keep NPLC consistent for each buffer */
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
        fEq->fOdbEqVariables->WD("Shutter Status", currentShutterStatus);   // also save as an ODB variable
        fEq->fOdbEqVariables->WD("Wavelength", currentWavelength);
        fEq->WriteStatistics(); // update the statistics like number of events sent, etc

        // update status on MIDAS Status page
        /*char msg[64] = {0};
        sprintf(msg, "%.2f V, %.3e A", volt, mean);
        fEq->SetStatus(msg, "lightgreen");*/
    }
};

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
