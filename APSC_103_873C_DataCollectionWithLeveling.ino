/*
Code Created by group 873 - C
Coded with MPU9255 in mind,
Look for: SD card initialization failed!, try putting the sd card in again as data will not be collected

THIS IS THE MOST UP-TO-DATE CODE
*/
#include <SPI.h>
#include <SD.h>
#include <MPU9255.h>
#include <math.h> 
 
//SD card setup 
const int chipSelect = 53;
File RailSensorData;
unsigned long lastFlushTime = 0;
const unsigned long flushInterval = 100; // ms
 
//Encoder setup
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
float gyroX_bias = 0, gyroY_bias = 0;
float alpha = 0.98;
unsigned long lastTime;
float roll = 0; // roll angle in degrees
 
void setup() {
  //Set your baund to 9600 for data to be collected correctly
  Serial.begin(9600);
 
  // Encoder
  pinMode(ENCODER_CLK, INPUT);
  lastClk = digitalRead(ENCODER_CLK);
 
  // SD card setup
  pinMode(chipSelect, OUTPUT);
  digitalWrite(chipSelect, HIGH); // deselect
 
 //If the SD card cannot be initialized correct and pointer is null then
 //This error message is printed in the serial monitor, try fiddling withe
 //SD card as it may be an issue how it was plugged in
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }
 
  // Open CSV and keep it open
  RailSensorData = SD.open("Rail.csv", FILE_WRITE);

  //Look for this error message in the serial monitor and 
  //check if the csv is in the file you are running this in
  if (!RailSensorData) {
    Serial.println("Failed to open file!");
    while (1);
  }
 
  // Write header
  RailSensorData.println("Time,Pulses,Distance,Speed,AX,AY,AZ,GX,GY,GZ,Roll");
  RailSensorData.flush(); 
  Serial.println("Calibrating");

  //CODE FOR CALIBRATING THE LEVEL
  long sumX = 0, sumY = 0;
  int samples = 1000;
  lastTime = millis();
  for (int i = 0; i < samples; i++) {
    mpu.read_gyro(); // Updates mpu.gx, mpu.gy, mpu.gz
    sumX += mpu.gx;
    sumY += mpu.gy;
    delay(2);
  }
  gyroX_bias = (float)sumX / samples;
  gyroY_bias = (float)sumY / samples;
 
  Serial.println("Calibration Complete.");
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
 
    //Calculate roll
    float accRoll = atan2(mpu.ay, mpu.az) * 180.0 / M_PI;
    roll = alpha * (roll + (mpu.gy-gyroY_bias)*dt) + (1 - alpha) * accRoll;
 
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
    RailSensorData.println(roll);
 
  }
 
  // Flush periodically to ensure sensor is not clogged and
  //results in data being read incorrectly
  if (millis() - lastFlushTime >= flushInterval) {
    RailSensorData.flush();
    lastFlushTime = millis();
  }
}