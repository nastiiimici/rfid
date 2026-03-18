#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <map>

#define SS_PIN 5
#define RST_PIN 4
#define BUZZER_PIN 13

const char* ssid = "nastimici";
const char* password = "nastiiimiciii";

MFRC522 mfrc522(SS_PIN, RST_PIN);
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

std::map<String,String> uidNames; // UID -> jméno z API

void beep(int t){
  digitalWrite(BUZZER_PIN,HIGH);
  delay(t);
  digitalWrite(BUZZER_PIN,LOW);
}

// vrátí čas HH:MM:SS
String getTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) return "cas?";
  char buffer[30];
  strftime(buffer,30,"%H:%M:%S",&timeinfo);
  return String(buffer);
}

// zobrazuje jméno + stav + čas na 1 sekundu
void showUID(String uid, bool stav, String cas){
  String name = uidNames.count(uid) ? uidNames[uid] : uid;
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(0,15,name.c_str());
  display.setFont(u8g2_font_ncenB14_tr);
  display.drawStr(0,35,stav?"ODCHOD":"PRICHOD");
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(0,55,cas.c_str());
  display.sendBuffer();
}

// zobrazuje pouze hodiny
void showClock(){
  String cas = getTime();
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB14_tr);
  display.drawStr(20,40,cas.c_str());
  display.sendBuffer();
}

// pošle UID na server a server aktualizuje stav
void sendToServer(String uid){
  if(WiFi.status()!=WL_CONNECTED) return;

  HTTPClient http;
  http.begin("http://130.61.200.245:20002/rfid");
  http.addHeader("Content-Type","application/json");
  String payload = "{\"uid\":\""+uid+"\"}";
  int code = http.POST(payload);
  if(code>0){
    Serial.print("HTTP Response: ");
    Serial.println(code);
  }
  else{
    Serial.print("HTTP Error: ");
    Serial.println(code);
  }
  http.end();
}

// stáhne aktuálně aktivní karty ze serveru
bool getCardState(String uid){
  if(WiFi.status()!=WL_CONNECTED) return false;

  HTTPClient http;
  http.begin("http://130.61.200.245:20002/active");
  int code = http.GET();
  if(code==200){
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc,payload);
    JsonArray arr = doc.as<JsonArray>();
    for(JsonVariant v : arr){
      if(uid.equalsIgnoreCase(v.as<const char*>())) return false; // aktivní = ODCHOD
    }
  }
  http.end();
  return true; // pokud není v seznamu, pak PŘÍCHOD
}

// načte UID -> jména ze serveru
void fetchUIDNames(){
  if(WiFi.status()!=WL_CONNECTED) return;

  HTTPClient http;
  http.begin("http://130.61.200.245:20002/uids");
  int code = http.GET();
  if(code==200){
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc,payload);
    for(JsonPair kv : doc.as<JsonObject>()){
      uidNames[kv.key().c_str()] = kv.value().as<const char*>();
    }
  } else {
    Serial.print("Chyba při načítání UID jmen: "); Serial.println(code);
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN,OUTPUT);

  SPI.begin();
  mfrc522.PCD_Init();

  Wire.begin(21,22);
  display.begin();

  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(5,20,"RFID dochazka");
  display.drawStr(5,40,"Pripojuji WiFi");
  display.sendBuffer();

  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED) delay(500);

  configTime(0,3600,"pool.ntp.org","time.nist.gov"); // GMT+1

  fetchUIDNames(); // načíst jména ze serveru

  Serial.println("RFID system ready");
}

void loop() {
  if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
    String uid = "";
    for(byte i=0;i<mfrc522.uid.size;i++){
      if(mfrc522.uid.uidByte[i]<0x10) uid+="0";
      uid += String(mfrc522.uid.uidByte[i],HEX);
    }
    uid.toUpperCase();

    // pošle stav na server
    sendToServer(uid);

    // stáhne stav ze serveru
    bool stav = getCardState(uid);

    String cas = getTime();
    Serial.print(uid); Serial.print(" ");
    Serial.print(stav?"ODCHOD ":"PRICHOD "); Serial.println(cas);

    beep(80);

    showUID(uid,stav,cas);
    delay(2000);
  }

  // stále hodiny
  showClock();
  delay(200);
}