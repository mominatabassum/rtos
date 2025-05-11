#include "imu.h"

#define IMU 0x69  
#define MAG_AK09916 0x0C 
#define ACCEL_XOUT_H 0x2D  
#define GYRO_XOUT_H 0x33  
#define REG_PWR_MGMT_1 0x06   
#define REG_INT_PIN_CFG 0x0F  

imu::imu(String x) : name(x) {
    init(true);
}

void imu::init(bool initarr) {
    initializeICM20948();
    initializeMagnetometer();

    if (initarr) {
        for (int i = 0; i < siz; i++) {
          ax_arr[i] = 0;
          ay_arr[i] = 0;
          az_arr[i] = 0;

          gx_arr[i] = 0;
          gy_arr[i] = 0;
          gz_arr[i] = 0;

          mx_arr[i] = 0;
          my_arr[i] = 0;
          mz_arr[i] = 0;
        }
    }
}

float imu::lpf(float val, float* arr) {

    arr[index] = val;

    index++;
    if (index == siz) { 
        index = 0; 
    }

    float sum = 0;
    for (int i = 0; i < siz; i++) { 
        sum += arr[i]; 
    }

    return sum / siz;
}


void imu::update() {
    accelX = readRegister16(IMU, ACCEL_XOUT_H);
    accelY = readRegister16(IMU, ACCEL_XOUT_H + 2);
    accelZ = readRegister16(IMU, ACCEL_XOUT_H + 4);
    gyroX = readRegister16(IMU, GYRO_XOUT_H);
    gyroY = readRegister16(IMU, GYRO_XOUT_H + 2);
    gyroZ = readRegister16(IMU, GYRO_XOUT_H + 4);

    Wire.beginTransmission(MAG_AK09916);
    Wire.write(0x10);
    Wire.endTransmission();
    Wire.requestFrom(MAG_AK09916, 1);
    uint8_t magStatus1 = Wire.read();

    if (magStatus1 & 0x01) {
        uint8_t magData[8];
        Wire.beginTransmission(MAG_AK09916);
        Wire.write(0x11);
        Wire.endTransmission();
        Wire.requestFrom(MAG_AK09916, 8);
        for (int i = 0; i < 8; i++) {
            magData[i] = Wire.read();
        }
        magX = (magData[1] << 8 | magData[0]);
        magY = (magData[3] << 8 | magData[2]);
        magZ = (magData[5] << 8 | magData[4]);
        uint8_t magStatus2 = magData[6];
        if (magStatus2 & 0x08) {
            init(false);
        }
    } else {
        init(false);
        magX = magY = magZ = 0;
    }



    accelX = lpf(accelX, ax_arr);
    accelY = lpf(accelY, ay_arr);
    accelZ = lpf(accelZ, az_arr); // Added missing lpf call for accelZ
    gyroX = lpf(gyroX, gx_arr);   // Added missing lpf call for gyroX
    gyroY = lpf(gyroY, gy_arr);   // Added missing lpf call for gyroY
    gyroZ = lpf(gyroZ, gz_arr);   // Added missing lpf call for gyroZ
    magX = lpf(magX, mx_arr);     // Added missing lpf call for magX
    magY = lpf(magY, my_arr);     // Added missing lpf call for magY
    magZ = lpf(magZ, mz_arr);     // Added missing lpf call for magZ

}

float* imu::getData() {
    update();
    static float data[9];
    data[0] = accelX;
    data[1] = accelY;
    data[2] = accelZ;
    data[3] = gyroX;
    data[4] = gyroY;
    data[5] = gyroZ;
    data[6] = magX;
    data[7] = magY;
    data[8] = magZ;
    return data;
}

String imu::getDataString() {
    update();
    String dataString = name + ":";
    dataString += String(accelX) + ",";
    dataString += String(accelY) + ",";
    dataString += String(accelZ) + ",";
    dataString += String(gyroX) + ",";
    dataString += String(gyroY) + ",";
    dataString += String(gyroZ) + ",";
    dataString += String(magX) + ",";
    dataString += String(magY) + ",";
    dataString += String(magZ);
    return dataString;
}

void imu::initializeICM20948() {
    Wire.beginTransmission(IMU);
    Wire.write(REG_PWR_MGMT_1);
    Wire.write(0x01);
    Wire.endTransmission();
    delay(100);
    Wire.beginTransmission(IMU);
    Wire.write(REG_INT_PIN_CFG);
    Wire.write(0x02);
    Wire.endTransmission();
}

void imu::initializeMagnetometer() {
    Wire.beginTransmission(MAG_AK09916);
    Wire.write(0x31);
    Wire.write(0x08);
    Wire.endTransmission();
    delay(10);
}

int16_t imu::readRegister16(uint8_t addr, uint8_t reg) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(addr, 2);
    if (Wire.available() < 2) return 0;
    return (Wire.read() << 8) | Wire.read();
}
