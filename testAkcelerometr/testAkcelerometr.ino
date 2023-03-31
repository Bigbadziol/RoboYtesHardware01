// Basic demo for accelerometer readings from Adafruit MPU6050

//TO
// ESP32 Guide: https://RandomNerdTutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/
// ESP8266 Guide: https://RandomNerdTutorials.com/esp8266-nodemcu-mpu-6050-accelerometer-gyroscope-arduino/
// Arduino Guide: https://RandomNerdTutorials.com/arduino-mpu-6050-accelerometer-gyroscope/

//poziomica
//https://www.youtube.com/watch?v=232jer4HIZc
//https://github.com/DesignBuildDestroy/digital_spirit_level/blob/main/Digital_Level_v1.ino
//https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050
//0x68
//0x72
//if (!mpu.begin(YOUR_I2C_ADDRESS)) {}
/* Get all possible data from MPU6050
 * Accelerometer values are given as multiple of the gravity [1g = 9.81 m/s²]
 * Gyro values are given in deg/s
 * Angles are given in degrees
 * Note that X and Y are tilt angles and not pitch/roll.
 *
 * License: MIT
 */
#include "Wire.h"
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

long timer = 0;

void kalibracjaUstawienia() {
    Serial.printf("AKCE. OFSETY : x:%.2f , y:%.2f , z:%.2F \n", mpu.getAccXoffset(), mpu.getAccYoffset(), mpu.getAccZoffset());
    Serial.printf("ZYRO. OFSETY : x:%.2f , y:%.2f , z:%.2F \n", mpu.getGyroXoffset(), mpu.getGyroYoffset(), mpu.getGyroZoffset());
    Serial.printf("AKCELEROMETR FILTR : %f \n", mpu.getFilterAccCoef());
    Serial.printf("ZYROSKOP FILTR : %f \n", mpu.getFilterGyroCoef());
};

void setup() {
    Serial.begin(115200);
    Wire.begin(); //Pamietac!! do glownego

    byte status = mpu.begin();
    Serial.print(F("MPU6050 status: "));
    Serial.println(status);
    while (status != 0) {} // stop everything if could not connect to MPU6050

    Serial.println(F("Przed kalibracja : "));
    kalibracjaUstawienia();
    delay(1000);
    mpu.calcOffsets(true, true); // gyro and accelero
    Serial.println("\n\n");
    kalibracjaUstawienia();

}

void loop() {
    mpu.update();

    if (millis() - timer > 1000) { // print data every second
        Serial.print(F("TEMPERATURE: ")); Serial.println(mpu.getTemp());
        Serial.print(F("ACCELERO  X: ")); Serial.print(mpu.getAccX());
        Serial.print("\tY: "); Serial.print(mpu.getAccY());
        Serial.print("\tZ: "); Serial.println(mpu.getAccZ());

        Serial.print(F("GYRO      X: ")); Serial.print(mpu.getGyroX());
        Serial.print("\tY: "); Serial.print(mpu.getGyroY());
        Serial.print("\tZ: "); Serial.println(mpu.getGyroZ());

        Serial.print(F("ACC ANGLE X: ")); Serial.print(mpu.getAccAngleX());
        Serial.print("\tY: "); Serial.println(mpu.getAccAngleY());

        Serial.print(F("ANGLE     X: ")); Serial.print(mpu.getAngleX());
        Serial.print("\tY: "); Serial.print(mpu.getAngleY());
        Serial.print("\tZ: "); Serial.println(mpu.getAngleZ());
        Serial.println(F("=====================================================\n"));
        timer = millis();
    }

}