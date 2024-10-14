#include <cstring> // strncpy 사용을 위해 필요
#include <iostream>

using namespace std;

class User
{
private:
    long user_id;
    string uni_num;
    double user_x;
    double user_y;
    double user_dist;
    double user_lat;
    double user_lon;
    bool user_flag;

public:
    User(long id, const std::string &num, double dist, bool flag) : user_id(id), uni_num(num), user_dist(dist), user_flag(flag) {}

    long getUserId() const { return user_id; }
    void setUserId(long id) { user_id = id; }

    // uni_num
    string getUniNum() const { return uni_num; }
    void setUniNum(const string &num)
    {
        uni_num = num;
    }

    // user_x
    double getUserX() const { return user_x; }
    void setUserX(double x) { user_x = x; }

    // user_y
    double getUserY() const { return user_y; }
    void setUserY(double y) { user_y = y; }

    // user_dist
    double getUserDist() const { return user_dist; }
    void setUserDist(double dist) { user_dist = dist; }

    // user_lat
    double getUserLat() const { return user_lat; }
    void setUserLat(double lat) { user_lat = lat; }

    // user_lon
    double getUserLon() const { return user_lon; }
    void setUserLon(double lon) { user_lon = lon; }

    double getUserFlag() const { return user_flag; }
    void setUserFlag(bool flag) { user_flag = flag; }
    
};