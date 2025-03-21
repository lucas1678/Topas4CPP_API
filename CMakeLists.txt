cmake_minimum_required(VERSION 3.10)
project(Topas4Locator)

# modify to access vcpkg.cmake in your system
include(C:/Users/backe/Downloads/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find or fetch nlohmann/json library
include(FetchContent)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# Find libcurl package
find_package(CURL REQUIRED)

# Common source files (shared between executables)
set(COMMON_SOURCES
    TopasLocator.cc
    TopasCommunicator.cc
    TopasDevice.cc
)

# First executable
add_executable(topas4_locate example.cc ${COMMON_SOURCES})

# Second executable  
add_executable(topas4_http_example http_example.cc ${COMMON_SOURCES})

# Include directories (for both executables)
target_include_directories(topas4_locate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CURL_INCLUDE_DIRS})
target_include_directories(topas4_http_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CURL_INCLUDE_DIRS})

# Link libraries (for both executables)
target_link_libraries(topas4_locate PRIVATE nlohmann_json::nlohmann_json ${CURL_LIBRARIES})
target_link_libraries(topas4_http_example PRIVATE nlohmann_json::nlohmann_json ${CURL_LIBRARIES})

# Platform-specific settings (for both executables)
if(WIN32)
  target_link_libraries(topas4_locate PRIVATE ws2_32)
  target_link_libraries(topas4_http_example PRIVATE ws2_32)

  # On Windows, add CURL_STATICLIB definition if using static curl
  if(CURL_STATIC_LIBRARY)
    target_compile_definitions(topas4_locate PRIVATE CURL_STATICLIB)
    target_compile_definitions(topas4_http_example PRIVATE CURL_STATICLIB)
  endif()
endif()

# Optional: Add installation rules for both executables
install(TARGETS topas4_locate topas4_http_example DESTINATION bin)