   //Library dan pin untuk BMP 280
#include <Wire.h>
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp; // I2C

//Library dan pin untuk LoRa
#include <SPI.h>
#include <LoRa.h>
#define ss 17
#define rst 16
#define dio0 15
#define BAND 915E6    //433E6 for Asia
 
//Library dan pin untuk DHT22
#include "DHT.h"
#define DHTPIN 12
DHT dht(DHTPIN, DHT22);

//packet counter
int readingID = 0;
 
int counter = 0;
String LoRaMessage = "";
 
float temperature = 0;
float humidity = 0;
int tekanan = 0;
int tekananmbar = 0;
 
//Start LORA
void startLoRA()
{
  LoRa.setPins(ss, rst, dio0); //setup LoRa transceiver module
 
  while (!LoRa.begin(BAND) && counter < 10) {
    Serial.print(".");
    counter++;
    delay(500);
  }
  if (counter == 20) 
  {
    // Increment readingID on every new reading
    readingID++;
    Serial.println("Starting LoRa failed!"); 
  }
  Serial.println("LoRa Initialization OK!");
  delay(2000);
}
 
//Start DHT 22
void startDHT()
{
  if (isnan(humidity) || isnan(temperature)) 
  {
  Serial.println("Failed to read from DHT sensor!");
  return;
  }
}

//Start BMP280
void startBMP(){
     while ( !Serial ) delay(100);   // wait for native usb
    Serial.println(F("BMP280 test"));
    unsigned status;
    status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
    //status = bmp.begin();
    if (!status) {
      Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
      Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
      Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("        ID of 0x60 represents a BME 280.\n");
      Serial.print("        ID of 0x61 represents a BME 680.\n");
      while (1) delay(10);
    }

  /* Default settings from datasheet. */
   bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  
  
  }

 
void getReadings(){
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  //Get Reading Untuk BMP280//
  tekanan = bmp.readPressure();
  tekananmbar = tekanan*0.0106;
  ////////////////////////////
  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.println(F(" % "));
  Serial.print(F("Temperature: "));
  Serial.print(temperature);
  Serial.println(F(" °C "));
  Serial.print(F("Tekanan: "));
  Serial.print(tekananmbar);
  Serial.print(F(" mbar "));
}
 
void sendReadings() {
  LoRaMessage = String(readingID) + " / " + String(temperature) + " & " + String(humidity) + " # " + String(tekananmbar);//TARUH DISINI//
  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print(LoRaMessage);
  LoRa.endPacket();
  
  Serial.print("Sending packet: ");
  Serial.println(readingID);
  readingID++;
  Serial.println(LoRaMessage);
}
 
void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  dht.begin();
  startDHT();
  startLoRA();
  startBMP();
}
void loop() {
  getReadings();
  sendReadings();
  delay(5000);
}
