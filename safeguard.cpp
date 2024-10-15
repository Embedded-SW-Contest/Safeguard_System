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
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <errno.h>
#include <wiringSerial.h> // C 라이브러리 호환
#include <future>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;
//-------------------------------------------------------------------------

SSD1306::OledI2C oled{OLEDConfig::OLEDPort, OLEDConfig::OLEDAddress};
MicropyGPS gps;
int fd_gps;
Lidar lidar;
// vector<User> user_v;
double brakingDistance;
vector<User> user_v;
httplib::SSLClient cli(HttpConfig::Address);

int connectDevices()
{

    // 왼쪽 라이다의 시리얼 포트 열기
    if (lidar.openSerialPort(LidarConfig::LeftLidarPort, &(lidar.lPort)) == -1)
    {
        cerr << "Failed to open lPort." << endl;
        return -1;
    }

    //오른쪽 라이다의 시리얼 포트 열기 (GPIO : ttyAMA[] , USB : ttyUSB[])
    if (lidar.openSerialPort(LidarConfig::RightLidarPort, &lidar.rPort) == -1)
    {
        cerr << "Failed to open rPort." << endl;
        return -1;
    }

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


void getUserData()
{

    auto res = cli.Get("/api/users");

    if (res && (res->status == 200))
    {
        cout << "Status: " << res->status << endl;
        json j = json::parse(res->body);
        user_v.clear();

        if (j.is_array())
        {
            for (const auto &item : j)
            {
                long user_id = item["user_id"].get<long>();
                std::string uni_num = item["uni_num"].get<std::string>();
                double user_dist = item["user_dist"].get<double>();
                bool user_flag = item["user_flag"].get<int>();
                cout<<"user_flag : "<<user_flag<<"flagflagflagflag";
                user_v.emplace_back(user_id, uni_num, user_dist,user_flag);
                
            }
        }
        else
        {
            long user_id = j["user_id"].get<long>();
            std::string uni_num = j["uni_num"].get<std::string>();
            double user_dist = j["user_dist"].get<double>();
            bool user_flag = j["user_flag"].get<int>();
            
             user_v.emplace_back(user_id, uni_num, user_dist,user_flag);
        }
    }
    else
    {
        auto err = res.error();
        std::cerr << "Request failed with error(getUserData): " << httplib::to_string(err) << "\n";
    }
}

void postCarData()
{
    // POST 요청에 보낼 JSON 데이터 생성
    json requestBody = {
        {"uni_num", "CAR2"},
        {"car_lat", gps.getLatitude()},
        {"car_lon", gps.getLongitude()},
        {"braking_distance", brakingDistance}
        };

    // JSON 데이터를 문자열로 변환
    std::string requestData = requestBody.dump();

    // 헤더 설정 (Content-Type을 JSON으로 지정)
    httplib::Headers headers = {
        {"Content-Type", "application/json"}};

    // POST 요청 보내기
    auto res = cli.Post("/api/cars", headers, requestData, "application/json");

    // 응답 확인
    if (res && (res->status == 200 || res->status == 201))
    {
        cout << "Status: " << res->status << endl;
        //cout << "Body: " << res->body << endl;
    }
    else
    {
        if (res)
        {
            cerr << "Request failed. Status:(PostCarData) " << res->status << endl;
        }
        else
        {
            auto err = res.error();
            cerr << "Request error:(PostCarData) " << httplib::to_string(err) << endl;
        }
    }
}
//---
future<void> futureGetData;
future<void> futurePostData;
bool isGetDataRunning = false;
bool isPostDataRunning = false;
void asyncHTTP()
{
    if (isGetDataRunning && futureGetData.valid() &&
        futureGetData.wait_for(chrono::seconds(0)) == future_status::ready)
    {
        futureGetData.get(); // 비동기 작업의 결과 가져오기
        isGetDataRunning = false;
    }

    // 이전 비동기 작업이 완료되었는지 확인 (POST)
    if (isPostDataRunning && futurePostData.valid() &&
        futurePostData.wait_for(chrono::seconds(0)) == future_status::ready)
    {
        futurePostData.get(); // 비동기 작업의 결과 가져오기
        isPostDataRunning = false;
    }

    // 새로운 비동기 GET 작업 실행
    if (!isGetDataRunning)
    {
        futureGetData = async(std::launch::async, getUserData);
        isGetDataRunning = true;
    }

    // 새로운 비동기 POST 작업 실행
    if (!isPostDataRunning)
    {
        futurePostData = async(std::launch::async, postCarData);
        isPostDataRunning = true;
    }
}
//---

void calculateBrakingDistance(){
    double speedKPH = gps.getSpeed();
    double speedMPS = speedKPH * 1000 / 3600;
    brakingDistance = speedMPS * driver::ReactionTime + pow(speedMPS,2)/(2*driver::accelMPSS);  
}

int main()
{
    try
    {

        lidar = Lidar();
        connectDevices();

        while (true)
        {

            getGPS();
            asyncHTTP();
            drawDisplay();
            
            lidar.getTFminiData(0, lidar.lPort);
            lidar.getTFminiData(1, lidar.rPort);
            lidar.showAll();

            if (lidar.getLeft() <= LidarConfig::LimitDistance || lidar.getRight() <= LidarConfig::LimitDistance)
            {
                calculateBrakingDistance();
                for(User user : user_v){        
                    if(user.getUserDist() < brakingDistance){ // && user.getFlag()
                        //경고 울림!
                        cout<<"*************경고!!*************";
                        //해당 user의 car_flag값을 true로 바꿔서 휴대폰에서도 울리게..
                        
                        
                    }
                    continue;
                }
               
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
