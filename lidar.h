#ifndef LIDAR_H

#include <iostream>
#include <libserialport.h>
#include <unistd.h>

using namespace std;



class Lidar{
private:
    double left_distance;
    double right_distance;
public:
    sp_port *lPort;
    sp_port *rPort;
    Lidar(){
        left_distance = 0.0;
        right_distance = 0.0;
        lPort = nullptr;
        rPort = nullptr;
    }
    double getLeft();
    double getRight();
    void setLeft(double l);
    void setRight(double r);
    void getTFminiData(int num, sp_port *port);
    //void getRighttTFminiData();
    void showAll();
    int openSerialPort(const char *portName, sp_port **port);
};


#endif
