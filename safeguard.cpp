#define CPPHTTPLIB_OPENSSL_SUPPORT // HTTPS 기능 강제 활성화


#include "OledFont8x16.h"
#include "OledI2C.h"
#include "gps.h"
#include "lidar.h"
#include "config.h"
#include "httplib.h" // restAPI 통신
#include "user.h"


// #include <vector>
// #include <algorithm>
#include <iostream>
#include <random>

#include <wiringSerial.h>  // C 라이브러리 호환

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <future>
#include <nlohmann/json.hpp>


using json = nlohmann::json;
using namespace std;
//-------------------------------------------------------------------------

SSD1306::OledI2C oled{OLEDConfig::OLEDPort, OLEDConfig::OLEDAddress};
MicropyGPS gps;
int fd_gps;
Lidar lidar;
//vector<User> user_v;

httplib::SSLClient cli(HttpConfig::Address);

int connectDevices()
{

    // 왼쪽 라이다의 시리얼 포트 열기
    if (lidar.openSerialPort(LidarConfig::LeftLidarPort, &(lidar.lPort)) == -1)
    {
        cerr << "Failed to open lPort." << endl;
        return -1;
    }

    // 오른쪽 라이다의 시리얼 포트 열기 (GPIO : ttyAMA[] , USB : ttyUSB[])
    // if (lidar.openSerialPort("/dev/ttyAMA0", &lidar.rPort) == -1)
    // {
    //     cerr << "Failed to open rPort." << endl;
    //     return -1;
    // }

    if ((fd_gps = serialOpen(GPSConfig::GpsPort, GPSConfig::GpsBaudRate)) < 0)
    {
        std::cerr << "Unable to open serial device: " << std::endl;
        return 1;
    }

    return 0;
}

void getGPS()
{
    int length = serialDataAvail(fd_gps);
    if (length > 0)
    {
        for (int i = 0; i < length; i++)
        {
            char new_char = serialGetchar(fd_gps); // 한 문자를 읽      // cout<<new_char<<"\n";
            gps.update(new_char);
        }
    }
}

void drawDisplay()
{
    drawString8x16(SSD1306::OledPoint{0, 0}, // 왼쪽 :좌우 , 오른쪽 : 위아래
                   "lat :" + gps.latitude_string() + "\n" +
                       "lon :" + gps.longitude_string() + "\n" +
                       "v :" + gps.speed_string(),
                   SSD1306::PixelStyle::Set,
                   oled);
    oled.displayUpdate();
}

void getData()
{
    auto res = cli.Get("/api/users");
    if (res)
    {
        // std::cout << "Status: " << res->status << "\n";
        // std::cout << "Body: " << res->body << "\n";
        json j = json::parse(res->body);

       if (j.is_array()) {
                for (const auto &item : j) 
                {
                    long user_id = item["user_id"].get<long>();
                    std::string uni_num = item["uni_num"].get<std::string>();
                    double user_dist = item["user_dist"].get<double>();
                    //if(){} user_dist가 일정거리 이내로 들어오면..
                    cout<<"user_dist : -------------------------------------------"<<user_dist<<"\n";
                    

                       
                }
            } else {
                // 단일 객체일 경우 처리
                long user_id = j["user_id"].get<long>();
                std::string uni_num = j["uni_num"].get<std::string>();
                double user_dist = j["user_dist"].get<double>();
                //if(){} user_dist가 일정거리 이내로 들어오면..
                
            }
        
    }
    else
    {
        auto err = res.error();
        std::cerr << "Request failed with error: " << httplib::to_string(err) << "\n";
    }
}


int main()
{
    try
    {

        lidar = Lidar();
        connectDevices();

        future<void> future;    // 비동기 작업을 위한 future 객체
        bool isRunning = false;    // 비동기 작업 실행 상태 관리

        while (true)
        {

            
            
            getGPS();

            // 이전 비동기 작업이 완료되었는지 확인
            if (isRunning && future.valid() &&
                future.wait_for(chrono::seconds(0)) == future_status::ready) {
                future.get();  // 비동기 작업의 결과 가져오기
                isRunning = false;  // 비동기 작업이 완료되었음을 표시
            }

            // 새로운 비동기 작업 실행
           if (!isRunning) {
                future = async(std::launch::async, getData);
                isRunning = true;  // 비동기 작업이 시작됨을 표시
            }


            // cout << "latitude :" << gps.latitude_string() << "\n";
            // cout << "ongitude :" << gps.longitude_string() << "\n";
            // cout << "speed :" << gps.speed_string() << "\n";

            drawDisplay();

            lidar.getTFminiData(0, lidar.lPort);
            lidar.getTFminiData(1, lidar.rPort);
            lidar.showAll();

            if (lidar.getLeft() <= LidarConfig::LimitDistance && lidar.getRight() <= LidarConfig::LimitDistance)
            {
                //현재속도로 제동거리 구하느 함수 만들어야함


            }

            usleep(100000);
        }

        // oled.clear();
        // oled.displayUpdate();
    }

    catch (std::exception &e)
    {
        if (lidar.lPort != nullptr)
        {
            //    sp_close(lidar.lPort);
            std::cerr << "Exception caught -> fail to connect lidar.lport  " << std::endl;
        }
        if (lidar.rPort != nullptr)
        {
            //    sp_close(lidar.rPort);
            std::cerr << "Exception caught -> faile to connect lidar.rport " << std::endl;
        }
        std::cerr << e.what() << "\n";
    }

    return 0;
}
