#include "imu.h"
#include "BluetoothSerial.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

BluetoothSerial SerialBT;
#define SDA_PIN 21
#define SCL_PIN 22

imu sensors[] = {
    imu("r_shoulder"),
    // imu("a"),
    // imu("b"),
    // imu("c"),
};
size_t num = sizeof(sensors) / sizeof(sensors[0]);

String data = "";                
SemaphoreHandle_t dataSemaphore; 


void collectIMUData(void *param) {
    while (true) {
        if (xSemaphoreTake(dataSemaphore, portMAX_DELAY) == pdTRUE) {
            data = ""; 
            for (int i = 0; i < num; i++) {
                data += sensors[i].getDataString();
                data += "\n";
            }
            xSemaphoreGive(dataSemaphore);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}

void transmitBluetooth(void *param) {
    while (true) {
        String localData = "";

        if (xSemaphoreTake(dataSemaphore, portMAX_DELAY) == pdTRUE) {
            localData = data;
            xSemaphoreGive(dataSemaphore);
        }

        localData += "sent";
        SerialBT.println(localData);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);
    Serial.begin(250000);
    delay(1000);
    SerialBT.begin("MoCap");

    dataSemaphore = xSemaphoreCreateMutex();
    if (dataSemaphore == NULL) {
        Serial.println("Failed to create semaphore!");
        while (1);
    }

    xTaskCreatePinnedToCore(collectIMUData, "IMU Data Collection", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(transmitBluetooth, "Bluetooth Transmit", 4096, NULL, 1, NULL, 0);
}

void loop() {
    // Empty since FreeRTOS handles tasks
}
