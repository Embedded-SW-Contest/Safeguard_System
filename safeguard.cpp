#define CPPHTTPLIB_OPENSSL_SUPPORT // HTTPS 기능 강제 활성화


#include "gps.h"
#include "lidar.h"
#include "config.h"
#include "httplib.h" // restAPI 통신
#include "user.h"



#include <iostream>
#include <random>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <errno.h>
#include <wiringSerial.h> // C 라이브러리 호환
#include <wiringPi.h>
#include <future>
#include <nlohmann/json.hpp>
#include <curl/curl.h>


#define VIBRATION_PIN 0  // GPIO 17에 해당 (wiringPi 핀 번호 체계)

using json = nlohmann::json;
using namespace std;
//-------------------------------------------------------------------------

MicropyGPS gps;
int fd_gps;
Lidar lidar;
double brakingDistance;
vector<User> user_v;
httplib::SSLClient cli(HttpConfig::Address);

std::thread sse_thread;
std::atomic<bool> sse_running{true};

bool carFlag;

int connectDevices()
{
    if(wiringPiSetup() == -1)
    {
        std::cerr<<"wiringPi 초기화 실패!"<<std::endl;
        return -1;
    }

    pinMode(VIBRATION_PIN,OUTPUT);
    

    // 왼쪽 라이다의 시리얼 포트 열기
    if (lidar.openSerialPort(LidarConfig::LeftLidarPort, &(lidar.lPort)) == -1)
    {
        cerr << "Failed to open lPort." << endl;
        return -1;
    }

    // 오른쪽 라이다의 시리얼 포트 열기 (GPIO : ttyAMA[] , USB : ttyUSB[])
    if (lidar.openSerialPort(LidarConfig::RightLidarPort, &lidar.rPort) == -1)
    {
        cerr << "Failed to open rPort." << endl;
        return -1;
    }
    while (true)
    {
        if((fd_gps = serialOpen(GPSConfig::GpsPort, GPSConfig::GpsBaudRate)) > 0)
        {break;}
        std::cerr << "Unable to open serial device: " << std::endl;
        return 1;
    }
    cout<<"fd_gps : " << fd_gps <<"******************************************\n";

    return 0;
}

void getGPS()
{
    int length = serialDataAvail(fd_gps);
    cout<<"length : " << length<<"\n";
    if (length > 0)
    {
        for (int i = 0; i < length; i++)
        {
            char new_char = serialGetchar(fd_gps); 
            cout<<new_char;
            gps.update(new_char);
        }
        cout<<"\n";
    }
}




void calculateBrakingDistance()
{
    double speedKPH = gps.getSpeed();
    double speedMPS = speedKPH * 1000 / 3600;
    brakingDistance = speedMPS * driver::ReactionTime + pow(speedMPS, 2) / (2 * driver::accelMPSS);
   
}

size_t onReceiveData(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    std::string data(ptr, size * nmemb); // 받은 데이터 추출

    // "data: "로 시작하는 SSE 메시지를 추출
    size_t pos = data.find("data: ");


    if (pos != std::string::npos)
    {
        std::string json_str = data.substr(pos + 6); // "data: " 이후의 JSON 추출
        try
        {
            auto j = nlohmann::json::parse(json_str); // JSON 파싱
            user_v.clear();                           // 기존 데이터 초기화

            if (j.is_array())
            {
                for (const auto &item : j)
                {
                    long user_id = item["user_id"].get<long>();
                    std::string uni_num = item["uni_num"].get<std::string>();
                    double user_dist = item["user_dist"].get<double>();
                    bool user_flag = item["user_flag"].get<int>();

                    std::cout << "user_flag: " << user_flag << std::endl;
                    user_v.emplace_back(User{user_id, uni_num, user_dist, user_flag});
                }
            }
            else
            {
                long user_id = j["user_id"].get<long>();
                std::string uni_num = j["uni_num"].get<std::string>();
                double user_dist = j["user_dist"].get<double>();
                bool user_flag = j["user_flag"].get<int>();

                std::cout << "user_flag: " << user_flag << std::endl;
                user_v.emplace_back(User{user_id, uni_num, user_dist, user_flag});
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }

    return size * nmemb; // curl에 데이터 처리가 완료되었음을 알림
}
// SSE 연결 처리 함수
void connectToSSE(const std::string& url) {
    while (sse_running) {
        try {
            CURL* curl = curl_easy_init();
            if (!curl) {
                throw std::runtime_error("Failed to initialize CURL");
            }

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());  // URL 설정
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onReceiveData);  // 콜백 설정
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);  // 무한 대기
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);  // 리디렉션 허용

            std::cout << "Connecting to SSE stream..." << std::endl;

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
            }

            curl_easy_cleanup(curl);  // CURL 정리

            // 서버가 비정상적으로 끊겼을 때 재연결 시도
            std::this_thread::sleep_for(std::chrono::seconds(5));  // 5초 후 재시도
        } catch (const std::exception& e) {
            std::cerr << "Exception in SSE connection: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));  // 5초 후 재시도
        }
    }
}

void startSSE() {
    // SSE 스레드를 시작하는 함수
    try {
        std::cout << "Starting SSE thread..." << std::endl;
        connectToSSE(HttpConfig::full_Address_GetUser);
    } catch (const std::exception& e) {
        std::cerr << "SSE thread exception: " << e.what() << std::endl;
    }
}

void restartSSEIfNeeded() {
    // SSE 스레드가 종료되면 재시작하는 함수
    if (!sse_thread.joinable()) {  // 스레드가 종료된 경우
        sse_running = true;
        sse_thread = std::thread(startSSE);  // 새로운 스레드 시작
    }
}




void postCarData()
{
    if(carFlag){
        
    }
    // POST 요청에 보낼 JSON 데이터 생성
    json requestBody = {
        {"uni_num", "CAR"},
        {"car_lat", gps.getLatitude()},
        {"car_lon", gps.getLongitude()},
        {"braking_distance", brakingDistance},
        {"car_flag",carFlag}};

    // JSON 데이터를 문자열로 변환
    std::string requestData = requestBody.dump();

    // 헤더 설정 (Content-Type을 JSON으로 지정)
    httplib::Headers headers = {
        {"Content-Type", "application/json"}};

    // POST 요청 보내기
    auto res = cli.Post("https://uwb-safeguard.shop/api/cars", headers, requestData, "application/json");

  
    // 응답 확인
    if (res && (res->status == 200 || res->status == 201))
    {
        cout << "Status: " << res->status << endl;
          if(carFlag){
            usleep(2000000);
        }
        // cout << "Body: " << res->body << endl;
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






future<void> futurePostData;
bool isPostDataRunning = false;
void asyncHTTP()
{
    
    // 이전 비동기 작업이 완료되었는지 확인 (POST)
    if (isPostDataRunning && futurePostData.valid() &&
        futurePostData.wait_for(chrono::seconds(0)) == future_status::ready)
    {
        futurePostData.get(); // 비동기 작업의 결과 가져오기
        isPostDataRunning = false;
    }

 
    // 새로운 비동기 POST 작업 실행
    if (!isPostDataRunning)
    {
        futurePostData = async(std::launch::async, postCarData);
        isPostDataRunning = true;
    }
}



int main()
{
    try
    {

        lidar = Lidar();
        connectDevices();
        sse_thread = std::thread(startSSE);
        
        while (true)
        {   
            
            getGPS();
            carFlag = false;
            asyncHTTP();//
             cout<<"gpsLatitude : " << gps.getLatitude() << " gpsLongitude : "<<gps.getLongitude()<<"\n";
             cout<<"gps_speed : " <<gps.getSpeed()<<"\n";
       

            lidar.getTFminiData(0, lidar.lPort);
            lidar.getTFminiData(1, lidar.rPort);
            lidar.showAll();

            
            if (lidar.getLeft() <= LidarConfig::LimitDistance || lidar.getRight() <= LidarConfig::LimitDistance)
            {
                calculateBrakingDistance();
                digitalWrite(VIBRATION_PIN,LOW);
                for (User user : user_v)
                {

                    if (user.getUserDist() < brakingDistance && user.getUserFlag())
                    {
                        // 경고 울림!
                        cout << "*************경고!!*************";
                        carFlag = true;
                        postCarData();/
                        cout<<"carFlag: "<<carFlag<<"*********************\n";
                        digitalWrite(VIBRATION_PIN,HIGH);    
                        usleep(2000000);
                    }
                    continue;
                }
            }

            restartSSEIfNeeded();
            usleep(100000);
        }


    }

    catch (std::exception &e)
    {
        if (lidar.lPort != nullptr)
        {
            
            std::cerr << "Exception caught -> fail to connect lidar.lport  " << std::endl;
        }
        if (lidar.rPort != nullptr)
        {
           
            std::cerr << "Exception caught -> faile to connect lidar.rport " << std::endl;
        }
        std::cerr << e.what() << "\n";
    }

    return 0;
}


