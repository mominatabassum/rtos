#ifndef IMU_H
#define IMU_H

#include <Wire.h>
#include <Arduino.h>

class imu {

public:
    String name;
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;
    float magX, magY, magZ;

    int index;
    static const int siz = 10; // Length of the moving average window

    float ax_arr[siz], ay_arr[siz], az_arr[siz];
    float gx_arr[siz], gy_arr[siz], gz_arr[siz];
    float mx_arr[siz], my_arr[siz], mz_arr[siz];

    imu(String x);
    void update();
    float* getData();
    String getDataString();

private:
    void init(bool initarr);
    void initializeICM20948();
    void initializeMagnetometer();
    int16_t readRegister16(uint8_t addr, uint8_t reg);
    float lpf(float val, float* arr);
};

#endif
