#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <ctime>
#include <fstream>
#include <map>
#include "gps.h"

using namespace std;


    // Constructors
    MicropyGPS::MicropyGPS(int local_offset, string location_formatting)
    {
        this->local_offset = local_offset;
        this->coord_format = location_formatting;
        sentence_active = false;
        active_segment = 0;
        process_crc = false;
        crc_xor = 0;
        char_count = 0;
        fix_time = 0;

        _latitude = {0, 0.0, 'N'};
        _longitude = {0, 0.0, 'W'};
        speed = {0.0, 0.0, 0.0};
        course = 0.0;
        altitude = 0.0;
        geoid_height = 0.0;

        satellites_in_view = 0;
        satellites_in_use = 0;
        hdop = 0.0;
        pdop = 0.0;
        vdop = 0.0;
        valid = false;
        fix_stat = 0;
        fix_type = 1;

        crc_fails = 0;
        clean_sentences = 0;
        parsed_sentences = 0;
    }

    // Properties
    Coordinate MicropyGPS::latitude()
    {
        return _latitude;
    }

    Coordinate MicropyGPS::longitude()
    {
        return _longitude;
    }

    double MicropyGPS:: getLatitude(){
        double decimal_degrees = latitude().degrees + latitude().minutes/60;
        return decimal_degrees;
    }
    double MicropyGPS:: getLongitude(){ 
        double decimal_degrees = longitude().degrees + longitude().minutes/60;
        return decimal_degrees;
    }

    // Logging Functions
    bool MicropyGPS::start_logging(const string &target_file, const string &mode)
    {
        ios_base::openmode file_mode = (mode == "new") ? ios::out : ios::app;
        log_handle.open(target_file, file_mode);
        if (!log_handle.is_open())
        {
            cerr << "Invalid FileName" << endl;
            return false;
        }
        log_en = true;
        return true;
    }

    bool MicropyGPS::stop_logging()
    {
        if (log_handle.is_open())
        {
            log_handle.close();
        }
        else
        {
            cerr << "Invalid Handle" << endl;
            return false;
        }
        log_en = false;
        return true;
    }

    bool MicropyGPS::write_log(const string &log_string)
    {
        if (log_en && log_handle.is_open())
        {
            log_handle << log_string;
            return true;
        }
        return false;
    }

    void MicropyGPS::new_sentence()
    {
        // gps_segments 초기화: 첫 번째 세그먼트를 빈 문자열로 시작
        gps_segments.clear();       // 벡터 초기화
        gps_segments.push_back(""); // 첫 번째 세그먼트에 빈 문자열 추가

        // 나머지 변수들 초기화
        active_segment = 0;
        crc_xor = 0;
        sentence_active = true;
        process_crc = true;
        char_count = 0;
  
   
    }

    void MicropyGPS::update(char new_char)
    {
        
        bool valid_sentence = false;

        // Convert the new_char to its ASCII value
        int ascii_char = static_cast<int>(new_char);

        if (ascii_char >= 10 && ascii_char <= 126)
        {
            char_count++;

            // Write to log file if logging is enabled
            if (log_en)
            {
                write_log(string(1, new_char)); // Convert char to string and log
            }

            // Check if a new sentence is starting ('$')
            if (new_char == '$')
            {
                new_sentence();
                return;
            }

            if (sentence_active)
            {
                // Check if the sentence is ending ('*')
                if (new_char == '*')
                {
                    process_crc = false;
                    active_segment++;
                    gps_segments.push_back("");
                    return;
                }

                // Check if a section is ended (',')
                else if (new_char == ',')
                {
                    active_segment++;
                    gps_segments.push_back("");
                }

                // Store all other printable characters and update CRC if needed
                else
                {
                    gps_segments[active_segment] += new_char;

                    // CRC process
                    if (!process_crc && gps_segments[active_segment].length() == 2)
                    {
                        try
                        {
                            int final_crc = stoi(gps_segments[active_segment], nullptr, 16); // Convert to hex
                            if (crc_xor == final_crc)
                            {
                                valid_sentence = true;
                            }
                            else
                            {
                                crc_fails++;
                            }
                        }
                        catch (invalid_argument &)
                        {
                            // Invalid CRC value
                        }
                    }
                }

                // Update CRC
                if (process_crc)
                {
                    crc_xor ^= ascii_char;
                }

                // If a valid sentence was received and it's a supported sentence, parse it
                if (valid_sentence)
                {
                    clean_sentences++;
                    sentence_active = false;
                    if(gps_segments[0] == "GPRMC")
                        if(gprmc()){
                            parsed_sentences++;
                        }
                    return;
                    // if (supported_sentences.find(gps_segments[0]) != supported_sentences.end())
                    // {
                    //     // Parse the sentence
                    //     if (supported_sentences[gps_segments[0]](this))
                    //     {
                    //         parsed_sentences++;
                    //         return gps_segments[0];
                    //     }
                    // }
                }

                // Prevent the sentence buffer from filling with garbage
                if (char_count > SENTENCE_LIMIT)
                {
                    sentence_active = false;
                }
            }
        }
    }

    // Parsing functions (similar to Python methods like gprmc)
    bool MicropyGPS::gprmc()
    {
        // Check if gps_segments has enough elements before accessing them
        if (gps_segments.size() < 10)
        {
            cerr << "Error: gps_segments does not contain enough data." << endl;
            return false;
        }

        // UTC Timestamp
        try
        {
            if (!gps_segments[1].empty())
            {
                int hours = (stoi(gps_segments[1].substr(0, 2)) + local_offset) % 24;
                int minutes = stoi(gps_segments[1].substr(2, 2));
                float seconds = stof(gps_segments[1].substr(4));
                timestamp = {static_cast<double>(hours), static_cast<double>(minutes), seconds};
            }
            else
            {
                timestamp = {0.0, 0.0, 0.0};
            }
        }
        catch (invalid_argument &)
        {
            cerr << "Error: Invalid timestamp data." << endl;
            return false;
        }

        // Date stamp
        try
        {
            if (!gps_segments[9].empty())
            {
                int day = stoi(gps_segments[9].substr(0, 2));
                int month = stoi(gps_segments[9].substr(2, 2));
                int year = stoi(gps_segments[9].substr(4, 2));
                date = {day, month, year};
            }
            else
            {
                date = {0, 0, 0};
            }
        }
        catch (invalid_argument &)
        {
            cerr << "Error: Invalid date data." << endl;
            return false;
        }

        // Ensure gps_segments[2] is 'A' before proceeding
        if (gps_segments[2] == "A")
        {
            // Longitude / Latitude
            try
            {
                int lat_degs = stoi(gps_segments[3].substr(0, 2));
                double lat_mins = stod(gps_segments[3].substr(2));
                char lat_hemi = gps_segments[4][0];

                int lon_degs = stoi(gps_segments[5].substr(0, 3));
                double lon_mins = stod(gps_segments[5].substr(3));
                char lon_hemi = gps_segments[6][0];

                float spd_knt = stof(gps_segments[7]);
                float course_val = (!gps_segments[8].empty()) ? stof(gps_segments[8]) : 0.0;

                _latitude = {lat_degs, lat_mins, lat_hemi};
                _longitude = {lon_degs, lon_mins, lon_hemi};
                speed = {spd_knt, spd_knt * 1.151, spd_knt * 1.852};
                course = course_val;
                valid = true;
            }
            catch (invalid_argument &)
            {
                cerr << "Error: Invalid GPS data in GPRMC sentence." << endl;
                return false;
            }
        }
        else
        {
            // If invalid, reset the GPS data
            _latitude = {0, 0.0, 'N'};
            _longitude = {0, 0.0, 'W'};
            speed = {0.0, 0.0, 0.0};
            course = 0.0;
            valid = false;
        }

        return true;
    }

    string MicropyGPS::latitude_string(){
        string lat_string;
        if (coord_format == "dd") {
            int decimal_degrees = latitude().degrees + latitude().minutes/60;
            lat_string = to_string(decimal_degrees) + "' " + latitude().hemisphere;
        } else if (coord_format == "dms") {
            lat_string = "현재 dms는 지원 X, dd를 사용해주세요";
            //lat_string = std::to_string(latitude().degrees) + "° " + std::to_string(latitude().minutes) + "' " + std::to_string(latitude().minutes) + "\" " + std::to_string(latitude().hemisphere);
        } else {
            lat_string = "현재 ddm는 지원 X, dd를 사용해주세요";
            //lat_string = std::to_string(latitude().degrees) + "° " + std::to_string(latitude().minutes) + "' " + std::to_string(latitude().hemisphere);
        }
        return lat_string;
    }

    string MicropyGPS::longitude_string(){
          std::string lon_string;
        if (coord_format == "dd") {
            int decimal_degrees = longitude().degrees + longitude().minutes/60;
            lon_string = to_string(decimal_degrees) + "' " + longitude().hemisphere;
        } else if (coord_format == "dms") {
             lon_string = "현재 dms는 지원 X, dd를 사용해주세요";
            //lon_string = std::to_string(longitude[0]) + "° " + std::to_string(longitude[1]) + "' " + std::to_string(longitude[2]) + "\" " + std::to_string(longitude[3]);
        } else {
            lon_string = "현재 ddm는 지원 X, dd를 사용해주세요";
           // lon_string = std::to_string(longitude[0]) + "° " + std::to_string(longitude[1]) + "' " + std::to_string(longitude[2]);
        }
        return lon_string;
    }

    string MicropyGPS::speed_string(string unit){
        std::string speed_string;
        if (unit == "mph") {
            speed_string = std::to_string(speed[1]) + " mph";
        } else if (unit == "knot") {
            if (speed[0] == 1) {
                speed_string = std::to_string(speed[0]) + " knot";
            } else {
                speed_string = std::to_string(speed[0]) + " knots";
            }
        } else {
            speed_string = std::to_string(speed[2]) + " kph";
        }
        return speed_string;
    }

    double MicropyGPS::getSpeed(){
        return speed[2] ;
    }

