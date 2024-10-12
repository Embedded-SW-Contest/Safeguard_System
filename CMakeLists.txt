cmake_minimum_required(VERSION 2.6)
project(SafeguardSystem)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

#--------------------------------------------------------------------------

add_library(SSD1306 STATIC lib/FileDescriptor.cxx
						   lib/OledHardware.cxx
						   lib/OledPixel.cxx
						   lib/OledFont8x8.cxx
						   lib/OledFont8x12.cxx
						   lib/OledFont8x16.cxx
						   lib/OledGraphics.cxx
						   lib/OledI2C.cxx)

include_directories(${PROJECT_SOURCE_DIR}/lib)
link_directories(/usr/lib/aarch64-linux-gnu)

set(EXTRA_LIBS ${EXTRA_LIBS} HD44780Lcd)



#--------------------------------------------------------------------------
find_package(OpenSSL REQUIRED)


add_executable(safeguard safeguard.cpp gps.cpp lidar.cpp)
target_link_libraries(safeguard SSD1306)





#  라이브러리 찾기 및 링크


target_link_libraries(safeguard OpenSSL::SSL OpenSSL::Crypto)

find_library(WIRINGPI_LIB wiringPi REQUIRED)
#-------#
if(WIRINGPI_LIB)
    target_link_libraries(safeguard ${WIRINGPI_LIB})
else()
    message(FATAL_ERROR "wiringPi library not found")
endif()


find_library(LIBSERIALPORT_LIB serialport REQUIRED)
if(LIBSERIALPORT_LIB)
    target_link_libraries(safeguard ${LIBSERIALPORT_LIB})
else()
    message(FATAL_ERROR "LIBSERIALPORT library not found")
endif()

