/*
  Tds sensor to A0, 3.3V with esp32, 5V with Arduino Mega (either Voltage should be fine, refer to line 50 -tdsSens.setAref(3.3);- to change voltage), Gnd
  Ph sensor to A1, 5V, Gnd
  Temperature sensor to digital 2, 5V, Gnd
*/

#include <EEPROM.h>
#include "GravityTDS.h"
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TDS_SENSOR_PIN A0
#define PH_SENSOR_PIN A1 // the pH meter Analog output is connected with the Arduino's Analog
#define TEMP_SENSOR_PIN 2

#define GPS_RX 3
#define GPS_TX 4

#define ONE_WIRE_BUS 2 // Data wire is plugged into pin 2 on the Arduino

SoftwareSerial gpsSerial(GPS_RX, GPS_TX);

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature tempSens(&oneWire);
GravityTDS tdsSens;
TinyGPS gps;

unsigned long int avgValue;  //Store the average value of the sensor feedback
int buf[10], temp;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  gpsSerial.begin(9600);

  //for tds
  tdsSens.setPin(TDS_SENSOR_PIN);
  tdsSens.setAref(3.3);  //reference voltage on ADC, default 5.0V on Arduino UNO
  tdsSens.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  tdsSens.begin();  //initialization

  //for temp
  tempSens.begin();
}

void loop() {
  float temp = getTemp();
  float ph = getPh();
  float tds = getTds();
  String gps = "";

  //display information every time a new sentence is correctly encoded
  while (gpsSerial.available() > 0)
    if (gps.encode(gpsSerial.read()))
      gps = getGPSData();

  // if 5000 milliseconds pass and there are no characters coming in
  // over the software serial port, show a "No GPS detected" error
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("No GPS detected");
    while (true);
  }

  Serial.println(gps + "," + String(temp) + "," + String(ph) + "," + String(tds) + "\n");
  delay(500);
}

float getPh() {
  //get 10 sample value from the sensor for smooth the value
  for (int i = 0; i < 10; i++) {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }

  //sort the analog from small to large
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buf[i] > buf[j]) {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++)               //take the average value of 6 center sample
    avgValue += buf[i];
  float phMillivolt = (float)avgValue * 5.0 / 1024 / 6; //convert the analog into millivolt
  float phVal = 7 - (phMillivolt / 57.14);                  //convert the millivolt into pH value
  return phVal;
}

float getTemp() {
  // variables sent back: phValue, temperature, tdsValue
  sensors.requestTemperatures(); // Send the command to get temperature readings
  float tempVal = sensors.getTempCByIndex(0);
  return tempVal;
}

float getTds() {
  //temperature = readTemperature();  //add your temperature sensor and read it
  tdsSens.setTemperature(getTemp());  // set the temperature and execute temperature compensation
  tdsSens.update();  //sample and calculate
  float tdsVal = tdsSens.getTdsValue();  // then get the value, tds value is in ppm
  delay(1000);
  return tdsVal;
}

String getGPSData() {
  String timeStr, gpsData, hour, minute, second;

  if (gps.location.isValid()) {
    gpsData += "{lat:" + (gps.location.lat(), 6);
    gpsData += ",lon:" + (gps.location.lng(), 6);
  }
  else {
    gpsData += "{location: not available";
    gpsData += ",location: not available";
  }
  
  //format hour, minute, and second before assigning them; add a zero in front of values that are only 1 digit long (<10)
  hour = gps.time.hour() < 10 ? "0" + String(gps.time.hour()) : String(gps.time.hour())
  minute = gps.time.minute() < 10 ? "0" + String(gps.time.minute()) : String(gps.time.minute());
  second = gps.time.second() < 10 ? "0" + String(gps.time.second()) : String(gps.time.second());

  if (gps.date.isValid() && gps.time.isValid()) {
    timeStr = String(gps.date.year()) + "-" + String(gps.date.month()) + "-" + String(gps.date.day()) + " " + hour + ":" + minute + ":" + second;
      gpsData += ",time:" + timeStr + "}";
  }
  else {
    Serial.println("Not Available");
  }
  return gpsData;
}
