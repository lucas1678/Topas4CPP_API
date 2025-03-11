cmake_minimum_required(VERSION 3.10)
project(Topas4Locator)

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

# Source files
set(SOURCES
    main.cc
    TopasLocator.cc
)

# Header files
set(HEADERS
    TopasLocator.hh
)

# Add executable target
add_executable(topas4_locate ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(topas4_locate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries
target_link_libraries(topas4_locate PRIVATE nlohmann_json::nlohmann_json)

# Platform-specific settings
if(WIN32)
  target_link_libraries(topas4_locate PRIVATE ws2_32)
endif()

# Optional: Add installation rules
install(TARGETS topas4_locate DESTINATION bin)