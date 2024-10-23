#ifndef GPSTEST_H

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <ctime>
#include <fstream>
#include <map>

using namespace std;

struct Coordinate
{
    int degrees;
    double minutes;
    char hemisphere;
};

class MicropyGPS
{
public:
    // Max Number of Characters a valid sentence can be
    const int SENTENCE_LIMIT = 90;
    const vector<string> HEMISPHERES = {"N", "S", "E", "W"};
    const vector<string> DIRECTIONS = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};
    const vector<string> MONTHS = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    void set_gps_segments(const vector<string> &segments)
    {
        gps_segments = segments;
    }
    // Constructors
    MicropyGPS(int local_offset=0, string location_formatting="dd"); //123.467 도  형식으로 표현하게 설정

    // Properties
    Coordinate latitude();

    Coordinate longitude();

    double getLatitude();
    double getLongitude();

    // Logging Functions
    bool start_logging(const string &target_file, const string &mode = "append");


    bool stop_logging();
    


    bool write_log(const string &log_string);

    void new_sentence();
   

    void update(char new_char);
    
    // Parsing functions (similar to Python methods like gprmc)
    bool gprmc();

    string latitude_string();
    string longitude_string();
    string speed_string(string unit = "kph");

    double getSpeed();
      

private:
    vector<string> gps_segments;
    vector<double> timestamp = {0.0, 0.0, 0.0};
    vector<int> date = {0, 0, 0};

    Coordinate _latitude;
    Coordinate _longitude;
    string coord_format;
    vector<double> speed;
    double course;
    double altitude;
    double geoid_height;
    int satellites_in_view;
    int satellites_in_use;
    bool valid;
    int fix_stat;
    int fix_type;
    double hdop;
    double pdop;
    double vdop;
    int local_offset;

    // Logging-related
    ofstream log_handle;
    bool log_en = false;

    // CRC and sentence handling
    bool sentence_active;
    int active_segment;
    bool process_crc;
    int crc_xor;
    int char_count;
    long fix_time;

    int crc_fails;
    int clean_sentences;
    int parsed_sentences;
};

#endif
