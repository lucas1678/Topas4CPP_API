cmake_minimum_required(VERSION 3.10)
project(Topas4Locator)

# modify to access vcpkg.cmake in your system (look into alternatives)
include(C:/Users/backe/Downloads/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find or fetch nlohmann/json library
include(FetchContent)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2 # Adjust to the version you prefer
)
FetchContent_MakeAvailable(json)

# Find libcurl package
find_package(CURL REQUIRED)

# Source files
set(SOURCES
    main.cc
    TopasLocator.cc
    TopasCommunicator.cc
    TopasDevice.cc
)

# Header files
set(HEADERS
    TopasLocator.hh
    TopasCommunicator.hh
    TopasDevice.hh
)

# Add executable target
add_executable(topas4_locate ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(topas4_locate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CURL_INCLUDE_DIRS})

# Link libraries
target_link_libraries(topas4_locate PRIVATE nlohmann_json::nlohmann_json ${CURL_LIBRARIES})

# Platform-specific settings
if(WIN32)
  target_link_libraries(topas4_locate PRIVATE ws2_32)

  # On Windows, add CURL_STATICLIB definition if using static curl
  if(CURL_STATIC_LIBRARY)
    target_compile_definitions(topas4_locate PRIVATE CURL_STATICLIB)
  endif()
endif()

# Optional: Add installation rules
install(TARGETS topas4_locate DESTINATION bin)