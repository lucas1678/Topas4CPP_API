#include "TopasCommunicator.hh"

// Callback function for CURL to write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
        return newLength;
    } catch(std::bad_alloc& e) {
        // Handle memory problem
        return 0;
    }
}

TopasCommunicator::TopasCommunicator(const std::string& serialNum) : m_serialNum{serialNum}, m_initialized{false} {
    //  Initialize CURL for the entire instance (globally)
    if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK){
        printf("Failed to initialize CURL!");
        return;
    }

    //  Grab Topas devices using locator, and select the one with correct serialNum
    std::vector<json> avaiableDevices = m_locator.locate();
    for(const auto& device : avaiableDevices){
        if(device["SerialNumber"] == m_serialNum){
            m_baseAddress = device["PublicApiRestUrl_Version0"];
            m_initialized = true;
            std::cout << "Sucessfully initialized device with base address: " << m_baseAddress << std::endl;
            return;
        }
    }

    //  If no device found, throw a warning
    std::cerr << "[WARNING] Failed to find device with serial number " << m_serialNum << std::endl;
}

TopasCommunicator::~TopasCommunicator(){

}

bool TopasCommunicator::isInitialized() const {
    return m_initialized;
}

std::string TopasCommunicator::baseAddress() const {
    return m_baseAddress;
}

json TopasCommunicator::get(const std::string& url) const {
    //  Check if device is properly initialized
    if (!m_initialized){
        std::cerr << "[ERROR] Device not initialized!" << std::endl;
        return json();
    }

    std::string fullUrl = m_baseAddress + url;
    std::string response;

    //  Initialize CURL session and check for errors
    CURL* curl = curl_easy_init();
    if(!curl){
        std::cerr << "Failed to start CURL session!" << std::endl;
        return json();
    }

    //  Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());  // defines the full URL that we are writing to
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);  // defines the write callback function
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);  // defines the pointer that gets passed to the callback function

    CURLcode res = curl_easy_perform(curl);  // executes our request
    if(res!=CURLE_OK){
        std::cerr << "Failed to perform GET request!\nCURL error: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return json();
    }
    curl_easy_cleanup(curl);

    // Return an empty JSON object if no response
    if (response.empty()) {
        return json::object();
    }

    //  Parse the JSON response
    try{
        return json::parse(response);
    } catch(const std::exception& e){
        std::cerr << "Failed to parse JSON response. Error: " << e.what() << std::endl;
        return json();
    }
}

json TopasCommunicator::put(const std::string& url, const json& data) const {
    //  Check if device is properly initialized
    if (!m_initialized){
        std::cerr << "Error: Device not initialized!" << std::endl;
        return json();
    }

    //  Initialize CURL session and check for errors
    CURL* curl = curl_easy_init();
    if(!curl){
        std::cerr << "Failed to start CURL session!" << std::endl;
        return json();
    }

    std::string fullUrl = m_baseAddress + url;
    std::string response;
    std::string jsonStr = data.dump();

    //  Set up headers and set CURL options
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    //  Send the request!
    CURLcode res = curl_easy_perform(curl);
    if(res!=CURLE_OK){
        std::cerr << "Failed to perform PUT request!\nCURL error: " << curl_easy_strerror(res) << std::endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return json();
    }
    //Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Return an empty JSON object if no response
    if (response.empty()) {
        return json::object();
    }

    //  Parse the JSON response
    try{
        return json::parse(response);
    } catch(const std::exception& e){
        std::cerr << "Failed to parse JSON response. Error: " << e.what() << std::endl;
        return json();
    }
}

json TopasCommunicator::post(const std::string& url, const json& data) const {
    //  Check if device is properly initialized
    if (!m_initialized){
        std::cerr << "Error: Device not initialized!" << std::endl;
        return json();
    }

    //  Initialize CURL session and check for errors
    CURL* curl = curl_easy_init();
    if(!curl){
        std::cerr << "Failed to start CURL session!" << std::endl;
        return json();
    }

    std::string fullUrl = m_baseAddress + url;
    std::string response;
    std::string jsonStr = data.dump();

    //  Set up headers and set CURL options
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);



    //  Send the request!
    CURLcode res = curl_easy_perform(curl);
    if(res!=CURLE_OK){
        std::cerr << "Failed to perform POST request!\nCURL error: " << curl_easy_strerror(res) << std::endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return json();
    }
    //Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Return an empty JSON object if no response
    if (response.empty()) {
        return json::object();
    }

    //  Parse the JSON response
    try{
        return json::parse(response);
    } catch(const std::exception& e){
        std::cerr << "Failed to parse JSON response. Error: " << e.what() << std::endl;
        return json();
    }
}