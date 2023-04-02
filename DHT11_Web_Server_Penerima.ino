#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd (0x27, 16, 2);  


// Import Wi-Fi library
#include <WiFi.h>
const char* ssid = "monggo";
const char* pass = "";
WiFiClient client;
void startwifi() {
    WiFi.begin(ssid, pass);
    while (WiFi.status()!= WL_CONNECTED){
      delay(500);
      Serial.println("Connecting to WiFi..");
      }
    Serial.print ("Connected to ");
    Serial.println (ssid);
  }

//THINGSPEAK
#include <ThingSpeak.h>
const char *server = "api.thingspeak.com";
unsigned long myChannelNumber = 2055625;
const char *myWriteAPIKey = "QB01ZGMAO4Q7UW6M";
void startthingspeak() {
  ThingSpeak.begin(client);
  }

//FIREBASE
#include <FirebaseESP32.h>

#define FIREBASE_HOST "percobaan-skripsi-b11ef-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "21Yub1CmEcOv61n5H8kdJmvY7RjrybDzzARsNpIU"

FirebaseData fbdo;

void startfirebase() {
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
  }

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>
 
//Libraries for OLED Display
#include <Wire.h>
 
// Libraries to get time from NTP Server
 
//define the pins used by the LoRa transceiver module
#define ss 17
#define rst 16
#define dio0 15
 
#define BAND 915E6    //433E6 for Asia, 866E6 for Europe, 915E6 for North America
 
// Variables to save date and time
String formattedDate;
String day;
String hour;
String timestamp;
 
 
// Initialize variables to get and save LoRa data
int rssi;
String loRaMessage;
String temperature;
String humidity;
String tekanan;
String readingID;
 
// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return temperature;
  }
  else if(var == "HUMIDITY"){
    return humidity;
  }
  else if(var == "TIMESTAMP"){
    return timestamp;
  }
  else if (var == "RRSI"){
    return String(rssi);
  }
  return String();
}
 
//Initialize LoRa module
void startLoRA(){
  int counter;
 
  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0); //setup LoRa transceiver module
 
  while (!LoRa.begin(BAND) && counter < 10) {
    Serial.print(".");
    counter++;
    delay(500);
  }
  if (counter == 10) {
    // Increment readingID on every new reading
    Serial.println("Starting LoRa failed!"); 
  }
  Serial.println("LoRa Initialization OK!");
  delay(2000);
}
 
// Read LoRa packet and get the sensor readings
void getLoRaData() {
  //Serial.print("Lora packet received: ");
  // Read packet
  while (LoRa.available()) {
    String LoRaData = LoRa.readString();
    // LoRaData format: readingID/temperature&soilMoisture#batterylevel
    // String example: 1/27.43&654#95.34
    //Serial.print(LoRaData); 
    
    // Get readingID, temperature and moisture
    int pos1 = LoRaData.indexOf('/');
    int pos2 = LoRaData.indexOf('&');
    int pos3 = LoRaData.indexOf('#');
    readingID = LoRaData.substring(0, pos1);

    Serial.println ("Receiving...................... ");
    temperature = LoRaData.substring(pos1 +1, pos2);
    int suhuint= temperature.toInt();
    Serial.print("suhu = ");
    Serial.print(suhuint);
    Serial.println(" °C ");
    ThingSpeak.writeField(myChannelNumber, 1, suhuint, myWriteAPIKey); 
    Firebase.setInt(fbdo,"/Value/suhu", suhuint);
    
    humidity = LoRaData.substring(pos2+1, pos3);
    int humint= humidity.toInt(); 
    Serial.print("kelembaban = ");
    Serial.print(humint);
    Serial.println(" % ");
    ThingSpeak.writeField(myChannelNumber, 2, humint, myWriteAPIKey); 
    Firebase.setInt(fbdo,"/Value/kelembaban", humint);
    
    tekanan = LoRaData.substring(pos3+1, LoRaData.length());
    int tekananint= tekanan.toInt();
    Serial.print("tekanan = ");
    Serial.print(tekananint);
    Serial.println(" mbar ");
    ThingSpeak.writeField(myChannelNumber, 3, tekananint, myWriteAPIKey); 
    Firebase.setInt(fbdo,"/Value/tekanan", tekananint);   

    lcd.setCursor(0,0);
    lcd.print(suhuint); lcd.print("C, "); lcd.print(humint); lcd.print("%");

    lcd.setCursor(0,1);
    lcd.print(tekanan); lcd.print(" mbar");
  }
  // Get RSSI
  rssi = LoRa.packetRssi();
  Serial.print(" with RSSI ");    
  Serial.println(rssi);
}
 
void setup() { 
  // Initialize Serial Monitor
    Serial.begin(115200);
    startwifi();
    startfirebase();
    startLoRA();
    startthingspeak();
    lcd.init();
    
    } 
 
void loop() {
  // Check if there are LoRa packets available
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    getLoRaData();
  }
}
