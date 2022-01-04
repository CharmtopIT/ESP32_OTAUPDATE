#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"

const char * ssid = "CharmLinksys";
const char * password = "28511328";

int button_state;
int last_button_state; 


String FirmwareVer = {
  "2.0"
};
#define URL_fw_Version "https://raw.githubusercontent.com/CharmtopIT/ESP32_OTAUPDATE/master/esp32_ota/bin_version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/CharmtopIT/ESP32_OTAUPDATE/master/esp32_ota/fw.bin"
#define BP 23

void connect_wifi();
void firmwareUpdate();
int FirmwareVersionCheck();

unsigned long previousMillis = 0; // will store last time LED was updated
unsigned long previousMillis_2 = 0;
const long interval = 60000;
const long mini_interval = 1000;

void repeatedCall() {
  static int num=0;
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    if (FirmwareVersionCheck()) {
      firmwareUpdate();
    }
  }
  if ((currentMillis - previousMillis_2) >= mini_interval) {
    previousMillis_2 = currentMillis;
    Serial.print("idle loop...");
    Serial.print(num++);
    Serial.print(" Active fw version:");
    Serial.println(FirmwareVer);
   if(WiFi.status() == WL_CONNECTED) 
   {
       Serial.println("wifi connected");
   }
   else
   {
    connect_wifi();
   }
  }
}

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button_boot = {
  0,
  0,
  false
};
/*void IRAM_ATTR isr(void* arg) {
    Button* s = static_cast<Button*>(arg);
    s->numberKeyPresses += 1;
    s->pressed = true;
}*/

void IRAM_ATTR isr() {
  button_boot.numberKeyPresses += 1;
  button_boot.pressed = true;
}


void setup() {
  pinMode(BP, INPUT_PULLUP);
//  attachInterrupt(button_boot.PIN, isr, RISING);
  button_state = digitalRead(BP);
  Serial.begin(115200);
  Serial.print("Active firmware version:");
  Serial.println(FirmwareVer);
  pinMode(2, OUTPUT);
  connect_wifi();
}
void loop() {
  last_button_state = button_state;      // save the last state
  button_state = digitalRead(BP); // read new state
  if (last_button_state == HIGH && button_state == LOW) { //to connect wifi via Android esp touch app 
    Serial.println("The button is pressed");
    Serial.println("Firmware update Starting..");
    firmwareUpdate();
//    button_boot.pressed = false;
  }
  repeatedCall();
}

void connect_wifi() {
  Serial.println("Waiting for WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void firmwareUpdate(void) {
  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
  httpUpdate.setLedPin(2, LOW);
  t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

  switch (ret) {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    Serial.println("HTTP_UPDATE_OK");
    break;
  }
}
int FirmwareVersionCheck(void) {
  String payload;
  int httpCode;
  String fwurl = "";
  fwurl += URL_fw_Version;
  fwurl += "?";
  fwurl += String(rand());
  Serial.println(fwurl);
  WiFiClientSecure * client = new WiFiClientSecure;

  if (client) 
  {
    client -> setCACert(rootCACertificate);

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
    HTTPClient https;

    if (https.begin( * client, fwurl)) 
    { // HTTPS      
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      delay(100);
      httpCode = https.GET();
      delay(100);
      if (httpCode == HTTP_CODE_OK) // if version received
      {
        payload = https.getString(); // save received version
      } else {
        Serial.print("error in downloading version file:");
        Serial.println(httpCode);
      }
      https.end();
    }
    delete client;
  }
      
  if (httpCode == HTTP_CODE_OK) // if version received
  {
    payload.trim();
    if (payload.equals(FirmwareVer)) {
      Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
      return 0;
    } 
    else 
    {
      Serial.println(payload);
      Serial.println("New firmware detected");
      return 1;
    }
  } 
  return 0;  
}
