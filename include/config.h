#ifndef CONFIG_H
#define CONFIG_H

#include<iostream>


namespace LidarConfig {
    // Lidar 설정 값
    constexpr const char* LeftLidarPort = "/dev/ttyUSB0";
    constexpr const char* RightLidarPort = "/dev/ttyAMA2";
    static constexpr int LimitDistance = 1000;

  
}

namespace GPSConfig {
    // GPS 설정 값
    constexpr const char* GpsPort = "/dev/ttyAMA0";
    static constexpr int GpsBaudRate = 9600;

}

namespace OLEDConfig{
    constexpr const char* OLEDPort = "/dev/i2c-1";
    static constexpr int OLEDAddress  = 0x3C;
}

namespace HttpConfig{
    constexpr const char* Address = "uwb-safeguard.shop";
    constexpr const char* full_Address_GetUser ="https://uwb-safeguard.shop/api/users";
}

namespace driver{
    constexpr const char* car_name = "CAR";
    constexpr const double ReactionTime = 1.0;
    constexpr const double accelMPSS = 5;
    constexpr const char* websocket ="ws://localhost:8081";
}


#endif // CONFIG_H
