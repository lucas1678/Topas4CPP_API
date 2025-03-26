cmake_minimum_required(VERSION 3.10)
project(feLaserSystem)

# Show what $MIDASSYS is defined to:
message(STATUS "MIDAS frontends: MIDASSYS set to " $ENV{MIDASSYS})
if (NOT EXISTS $ENV{MIDASSYS})
  message(FATAL_ERROR "Environment variable $MIDASSYS not defined, aborting.")
endif()

set(INC_PATH $ENV{MIDASSYS}/include $ENV{MIDASSYS}/mxml $ENV{MIDASSYS}/mvodb $ENV{MIDASSYS}/mscb/include)
link_directories($ENV{MIDASSYS}/lib)

# ass required linux libraries
if (${CMAKE_SYSTEM_NAME} MATCHES Linux)
   set(LIBS ${LIBS} -lpthread -lutil -lrt)
   add_definitions(-DOS_LINUX)
endif()

add_library(mfe $ENV{MIDASSYS}/lib/mfe.o)
set_target_properties(mfe PROPERTIES LINKER_LANGUAGE CXX)

add_library(tmfe $ENV{MIDASSYS}/src/tmfe.cxx)
target_include_directories(tmfe PRIVATE ${INC_PATH})

add_library(tmfe0 OBJECT $ENV{MIDASSYS}/src/tmfe_rev0.cxx)
target_include_directories(tmfe0 PRIVATE ${INC_PATH})


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


set(SOURCES
    TopasLocator.cc
    TopasCommunicator.cc
    TopasDevice.cc
)


add_executable(feLaserSys TestFe.cc ${SOURCES})


target_include_directories(feLaserSys PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CURL_INCLUDE_DIRS} ${INC_PATH})

#  maybe add CURL_LIBRARIES to LIBS? same with nlohmann_json
target_link_libraries(feLaserSys nlohmann_json::nlohmann_json tmfe0 midas ${LIBS} ${CURL_LIBRARIES})
