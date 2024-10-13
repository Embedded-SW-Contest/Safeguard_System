#include "lidar.h"

double Lidar::getLeft(){
    return left_distance;
}
double Lidar::getRight(){
    return right_distance;
}
void Lidar::setLeft(double l){
    left_distance = l;
}
void Lidar::setRight(double r){
    right_distance = r;
}

void Lidar::getTFminiData(int num, sp_port *port) {

 
        int count = sp_input_waiting(port);  // sp_input_waiting 대신 sp_input_waiting 사용
 
        if (count > 8) {
            //cout << "--------------" << endl;
            unsigned char recv[9];
            sp_blocking_read(port, recv, 9, 1000);  // 시리얼 데이터 읽기
            sp_flush(port, SP_BUF_INPUT); // 버퍼 초기화
            
            
            if (recv[0] == 0x59 && recv[1] == 0x59) { // 0x59는 'Y'에 해당
                //cout << "distance : ";
                int low = recv[2]; // 추가적인 변환 없이 바로 값을 할당 O
                int high = recv[3];
                int distance = low + high * 256;
                
                //cout << distance << endl;
                if(num == 0){
                    setLeft(distance);
                }else{
                    cout<<"right";
                    setRight(distance);
                }
            }
        }

}

void Lidar::showAll(){
    cout << "left_distance : " << left_distance << endl;
    cout << "right_distance : " << right_distance << endl;
}

int Lidar::openSerialPort(const char *portName, sp_port **port) {
    sp_return result = sp_get_port_by_name(portName, port);
    if (result != SP_OK) {
        cerr << "Failed to find port: " << portName << endl;
        return -1;
    }

    result = sp_open(*port, SP_MODE_READ_WRITE);
    if (result != SP_OK) {
        cerr << "Failed to open port: " << portName << endl;
        return -1;
    }

    return 0;
}