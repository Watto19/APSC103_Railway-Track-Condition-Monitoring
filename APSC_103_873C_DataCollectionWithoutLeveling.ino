/*
Code Created by group 873 - C
This code was used while collecting on site data, coded with MPU9255 in mind,
Look for: SD card initialization failed!, try putting the sd card in again as data will not be collected
(EDIT: It has been iterated and changed along with a newly updated CSV file protocol
DO NOT USE THIS CODE FOR TESTING OF THE MACHINE IT IS NOT UP-TO-DATE)
*/

#include <SPI.h>
#include <SD.h>
#include <MPU9255.h>
#include <math.h>
 
// SD card setup
const int chipSelect = 53;
File RailSensorData;
unsigned long lastFlushTime = 0;
const unsigned long flushInterval = 100; // ms
 
// Encoder setup
#define ENCODER_CLK 2
double pulses = 0;
float distance;
const float clksize = 0.347; // inches per pulse
int newClk, lastClk;
unsigned long starttime = 0;
unsigned long currenttime;
float speed;
 
//MPU9255 setup
MPU9255 mpu;
float roll = 0; // roll angle in degrees
 
void setup() {
  Serial.begin(9600);
 
  // Encoder
  pinMode(ENCODER_CLK, INPUT);
  lastClk = digitalRead(ENCODER_CLK);
 
  // SD card setup
  pinMode(chipSelect, OUTPUT);
  digitalWrite(chipSelect, HIGH); 

  //This is a error message to be printed in the serial print
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }
 
  // Open CSV and keep it open

  RailSensorData = SD.open("Rail.csv", FILE_WRITE);
  if (!RailSensorData) {

    //Error message to look out for, data will not be outputted to an sd card 
    Serial.println("Failed to open file!");
    while (1);
  }
 
  // Write header
  RailSensorData.println("Time,Pulses,Distance,Speed,AX,AY,AZ,GX,GY,GZ,Roll");
  RailSensorData.flush();
}
 
void loop() {
  // Check encoder
  newClk = digitalRead(ENCODER_CLK);
  if (newClk != lastClk) {
    lastClk = newClk;
    pulses += 1;
    distance = pulses * clksize;
 
    // Calculate speed
    currenttime = micros();
    if (starttime == 0) starttime = currenttime;
    float dt = (currenttime - starttime) / 1000000.0; // seconds
    speed = (dt > 0) ? (clksize / dt) * 0.0568 : 0;    // mph
    starttime = currenttime;
 
    // Read accelerometer and gyroscope
    mpu.read_acc();
    mpu.read_gyro();
 
    // Write to SD card
    RailSensorData.print(millis());
    RailSensorData.print(",");
    RailSensorData.print(pulses);
    RailSensorData.print(",");
    RailSensorData.print(distance);
    RailSensorData.print(",");
    RailSensorData.print(speed);
    RailSensorData.print(",");
 
    RailSensorData.print(mpu.ax); RailSensorData.print(",");
    RailSensorData.print(mpu.ay); RailSensorData.print(",");
    RailSensorData.print(mpu.az); RailSensorData.print(",");
 
    RailSensorData.print(mpu.gx); RailSensorData.print(",");
    RailSensorData.print(mpu.gy); RailSensorData.print(",");
    RailSensorData.print(mpu.gz); RailSensorData.print(",");
 
  }
 
  // Flush periodically to prevent too much data 'clogging' the system

  if (millis() - lastFlushTime >= flushInterval) {
    RailSensorData.flush();

    //How it keeps track of how often the sensor should be flush
    lastFlushTime = millis();
  }
}