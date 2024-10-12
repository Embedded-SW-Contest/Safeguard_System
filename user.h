#include <cstring>  // strncpy 사용을 위해 필요
#include <iostream>

class User{
    private :
        long user_id;
        char uni_num[20];
        long user_x;
        long user_y;
        long user_dist;
        long user_lat;
        long user_lon;


    public :

    long getUserId() const { return user_id; }
    void setUserId(long id) { user_id = id; }

    // uni_num
    const char* getUniNum() const { return uni_num; }
    void setUniNum(const char* num) { 
        std::strncpy(uni_num, num, sizeof(uni_num) - 1); 
        uni_num[sizeof(uni_num) - 1] = '\0';  // 안전한 문자열 종료
    }

        // user_x
    long getUserX() const { return user_x; }
    void setUserX(long x) { user_x = x; }

    // user_y
    long getUserY() const { return user_y; }
    void setUserY(long y) { user_y = y; }

    // user_dist
    long getUserDist() const { return user_dist; }
    void setUserDist(long dist) { user_dist = dist; }

    // user_lat
    long getUserLat() const { return user_lat; }
    void setUserLat(long lat) { user_lat = lat; }

    // user_lon
    long getUserLon() const { return user_lon; }
    void setUserLon(long lon) { user_lon = lon; }


};