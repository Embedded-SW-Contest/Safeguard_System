#ifndef CONFIG_H
#define CONFIG_H

#include<iostream>


namespace LidarConfig {
    // Lidar 설정 값
    constexpr const char* LeftLidarPort = "/dev/ttyUSB0";
    constexpr const char* RightLidarPort = ""; //"/dev/ttyAMA0";
    static constexpr int LimitDistance = 1000;

  
}

namespace GPSConfig {
    // GPS 설정 값
    constexpr const char* GpsPort = "/dev/ttyAMA2";
    static constexpr int GpsBaudRate = 9600;

}

namespace OLEDConfig{
    constexpr const char* OLEDPort = "/dev/i2c-1";
    static constexpr int OLEDAddress  = 0x3C;
}

namespace HttpConfig{
    constexpr const char* Address = "00gym.shop";
}

namespace driver{
    constexpr const double ReactionTime = 1.0;
    constexpr const double accelMPSS = 5;
}


#endif // CONFIG_H