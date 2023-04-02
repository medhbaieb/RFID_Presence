
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
////////////////
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN D8
#define RST_PIN D0
String uid;


MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;
// Init array that will store new NUID
byte nuidPICC[4];
///////////////

ESP8266WiFiMulti WiFiMulti;

const char* ssid = "MSI";
const char* password = "1234567890";
int cpt=0;
String root = "https://rfidpresencev2.onrender.com/";
LiquidCrystal_I2C lcd(0x27, 16, 2);  



boolean setPresence(WiFiClient& client, HTTPClient& http, String code, boolean isPresent) {
  StaticJsonDocument<256> doc;

  if (http.begin(client, root + "set_presence")) {  // HTTP

    http.addHeader("Content-Type", "application/json");
    //Serial.print("[HTTP] PATCH...\n");
    // start connection and send HTTP header

    String request = "{\"code\":\"" + code + "\",\"is_present\":" + isPresent + "}";

    int httpCode = http.PATCH(request);
    // Serial.println(request);
    // httpCode will be negative on error
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        deserializeJson(doc, payload);  //srting to json
        Serial.println(payload);
        String message = (String)doc["message"];
        Serial.println(message);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    return true;
  }
  return false;
}

boolean getMemberByCode(WiFiClient& client, HTTPClient& http, String code, String& name ) {
  StaticJsonDocument<256> doc;
  boolean precense;
  if (http.begin(client, root + "get_user")) {  // HTTP

    http.addHeader("Content-Type", "application/json");
   // Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header

    String request = "{\"code\":\"" + code + "\"}";
    int httpCode = http.POST(request);
    Serial.println(httpCode);
    // Serial.println(request);
    // httpCode will be negative on error
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        deserializeJson(doc, payload);  //srting to json
        // Serial.println(payload);
        boolean success = (boolean)doc["success"];
        String message = (String)doc["message"];
        //Serial.println(message);
        if (success) {
          name = (String)doc["data"]["username"];
          precense=(boolean)doc["data"]["is_present"];
      
          if(precense==true){
            setPresence(client, http, code, false);
          }
          else{
            setPresence(client, http, code, true);
          }
          String message = precense?"   Good Bye":"    Welcome ";
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(message);
          lcd.setCursor(0,1);
          lcd.print(name);
          delay(2000);
          lcd.clear();
        } else {
          name = "";
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Member not found!");
          delay(2000);
          lcd.clear();
        }
      }
    } else {
      //Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    return true;
  }
  return false;
}




void setup() {
  //this the code of our project

  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  //this the code of our RFID
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522
  //Serial.println();
  //Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  lcd.setCursor(0,0);
  lcd.print("Enter your card");
}

void loop() {
  ///////////////////////////////////code of RFID
  uid = "";
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;
  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial())
    return;
  // Turn off LED
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  //Serial.println(rfid.PICC_GetTypeName(piccType));

 // Serial.print("Card UID gpt: ");
  for (byte i = 0; i < rfid.uid.size; i++) {

    //Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
  //  Serial.print(rfid.uid.uidByte[i], HEX);
    uid = uid + String(rfid.uid.uidByte[i], HEX);
  }
  Serial.println("   uidd  " + uid);
  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  /////////////////////////////////////////////////////////////////////////
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    Serial.println("connected");
    WiFiClient client;
    HTTPClient http;
    String name;
    
    ///////////////////////////////////////
    // if the cart is detected previously
    if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || rfid.uid.uidByte[3] != nuidPICC[3]) {
     // Serial.println(F("A new card has been detected."));
      for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = rfid.uid.uidByte[i];
      }
     
    } else {
     // Serial.println("Card read previously.");   
      
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Auth...");
    if (getMemberByCode(client, http, uid, name)) {
      Serial.println(name);
    } else {
      Serial.println("Wifi Error!");
    }
    lcd.setCursor(0,0);
    lcd.print("Enter your card");
  }
  
}


// function get member