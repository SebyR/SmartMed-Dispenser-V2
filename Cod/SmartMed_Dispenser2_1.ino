

//ESP32 Dev Module

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>    
#include "RTClib.h"             
#include <DFMiniMp3.h>
#include "EEPROM.h"
#include "ESP32_MailClient.h"
#include <Adafruit_INA219.h>
#include "Freenove_WS2812_Lib_for_ESP32.h"

Adafruit_INA219 ina219;

//const char* ssid = "BJPI PARTER";
//const char* password = "";

const char* ssid = "LSC";
const char* password = "ae3fa45bdf";

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#define EEPROM_SIZE 64

#define emailSenderAccount    "esp32.smdispenser@gmail.com"    
#define emailSenderPassword   "Ae!fa23bdf"
#define emailRecipient        "sebirosioru12@gmail.com"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "ALERTA"

#define LEDS_COUNT  1
#define LEDS_PIN  23
#define CHANNEL   0

const int dir = 2;
const int viteza = 4;                // step dir ptr motor
const int box = 34;                   // detectie o cutie
const int cup = 35;                   //detectie pahar
const int pinOk = 32;
const int pinUp = 14;
const int pinDn = 13;
const int cama = 15;
const int id = 33;
int stare = 0;
int pahar = 0;
int cont1 = 0;
int cont2 = 0;
int cont3 = 0;
int timp = 500;
int obs = 0;
int fault = 0;
int faults = 0;
int mail = 0;
int addr;
int meniu = 0;
int poz = 1;
int val = 30;
int load = 0;
int level = 0;
int radio = 0;
int pas = 0;
int distrib = 0;
int procente = 0;

SMTPData smtpData;
void sendCallback(SendStatus info);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS3231 rtc;
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_RGB);                   

class Mp3Notify
{
public:
  static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action)
  {
    if (source & DfMp3_PlaySources_Sd) 
    {
        Serial.print("SD Card, ");
    }
    if (source & DfMp3_PlaySources_Usb) 
    {
        Serial.print("USB Disk, ");
    }
    if (source & DfMp3_PlaySources_Flash) 
    {
        Serial.print("Flash, ");
    }
    Serial.println(action);
  }
  static void OnError(uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished(DfMp3_PlaySources source, uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(track);  
  }
  static void OnPlaySourceOnline(DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "online");
  }
  static void OnPlaySourceInserted(DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "inserted");
  }
  static void OnPlaySourceRemoved(DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "removed");
  }
};
DFMiniMp3<HardwareSerial, Mp3Notify> mp3(Serial2);

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  Serial.println();
  Serial.println();

  uint32_t currentFrequency;
  
  pinMode(dir, OUTPUT);
  pinMode(viteza, OUTPUT);
  pinMode(box, INPUT);
  pinMode(cup, INPUT);
  pinMode(pinOk, INPUT_PULLUP);
  pinMode(pinUp, INPUT_PULLUP);
  pinMode(pinDn, INPUT_PULLUP);
  pinMode(cama, INPUT);
  pinMode(id, INPUT);
  digitalWrite(dir, LOW);
  digitalWrite(viteza, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  EEPROM.begin(EEPROM_SIZE);
  display.display();
  mp3.begin();
  ina219.begin();
  strip.begin();
     
  display.clearDisplay();
  display.display();
  display.invertDisplay(false);

  mp3.setVolume(30);
  mp3.setEq(DfMp3_Eq_Rock);
  strip.setBrightness(254);
  
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));       //sincronizare data/ora
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("SmartMed"));
  display.println(F("Dispenser"));
  display.println(F("---V2---"));
  display.display();
  delay(2000);

 addr = EEPROM.read(60);
 display.clearDisplay();
 baterie();
}

void wireless(){
  WiFi.begin(ssid, password);   
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  radio = 0; 
}

void trimiteMail(){
 smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
 smtpData.setSender("URGENT", emailSenderAccount);
 smtpData.setPriority("High");
 smtpData.setSubject(emailSubject);
 smtpData.setMessage("Ana nu si-a luat medicamentele", true);
 smtpData.addRecipient(emailRecipient);
 smtpData.setSendCallback(sendCallback);
 
 if (!MailClient.sendMail(smtpData))
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());

  //Clear all data from Email object to free memory
  smtpData.empty();
  mail = 0; 
}

void waitMilliseconds(uint16_t msWait)
{
  uint32_t start = millis();
  
  while ((millis() - start) < msWait)
  {
    // calling mp3.loop() periodically allows for notifications 
    // to be handled without interrupts
    mp3.loop(); 
    delay(1);
  }
}

void msgIaPill(){
  digitalWrite(viteza, LOW);                          // ding si mesaj "ia pastilele"
  mp3.playFolderTrack(1, 5);
  waitMilliseconds(1000);
  strip.setLedColor(0, 255, 100, 0);
  mp3.playFolderTrack(1, 1);
  waitMilliseconds(5000);
  
}

void msgNuCup(){
   mp3.playFolderTrack(1, 5);
   waitMilliseconds(1000);
   strip.setLedColor(0, 255, 200, 0);
   mp3.playFolderTrack(1, 4);
   waitMilliseconds(5000);
}

void msgRogPill(){
  mp3.playFolderTrack(1, 5);
  waitMilliseconds(1000);
  strip.setLedColor(0, 255, 0, 0);
  mp3.playFolderTrack(1, 2);
  waitMilliseconds(5000);   
}

void msgThx(){
  mp3.playFolderTrack(1, 3);
  waitMilliseconds(1000);
  strip.setLedColor(0, 0, 0, 0); 
}

void ding(){
  mp3.playFolderTrack(1, 5);
  waitMilliseconds(1000); 
}

void baterie() 
{
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  procente = ina219.getBusVoltage_V() * 100;
  procente = map(procente, 280, 420, 0, 100);
  
  delay(100);
}

void loop() {
  pas++;
/*                                        //pentru debug
  Serial.print(cont1);
  Serial.print("  ");
  Serial.print(cont2);
  Serial.print("  ");
  Serial.print(cont3);
  Serial.println("  ");
  Serial.println(analogRead(cama));
  Serial.println(analogRead(id));
*/
  DateTime now = rtc.now();
  stare = analogRead(box);
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print(now.hour(), DEC);         
  display.print(F(":"));
  display.print(now.minute(), DEC);
  if (WiFi.status() == WL_CONNECTED){ 
   display.print(F("|"));        
  }
  if (WiFi.status() != WL_CONNECTED){
  display.print(F(" "));
  }

if (pas == 100){
   baterie();  
   pas = 0;  
}
  display.print(procente);
  display.println(F("%"));
  
  display.setTextSize(5);             
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,20);         
  display.print(F("> "));
  display.println(addr);        
  display.display();

 if(analogRead(id) < 1000 && meniu == 0){
  ding();
  strip.setLedColor(0, 0, 0, 128);
  meniu = 1;
  delay(1000);
 }

 while(meniu == 1){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
    display.print(poz);
    display.print(F(": "));
    display.println(val);
    display.display();
    if(digitalRead(pinUp) == LOW){
      val++;
      delay(200);
      if(val >= 59){val = 0;}
    }
    if(digitalRead(pinDn) == LOW){
      val--;
      delay(200);
      if(val <= 0){val = 0;}
    }
     if (digitalRead(pinOk) == LOW){
      addr = poz;
      EEPROM.write(addr, val);
      EEPROM.commit();
      delay(1000);
//      val = 0;
      poz++;
      if(poz > 4){poz = 1;}
  }
  if(analogRead(id) < 1000){
  ding();
  strip.setLedColor(0, 0, 0, 0);
  meniu = 0;
  addr = 1;
  delay(1000);
   }
 }

 if(mail == 2){
  if(radio == 0){
    wireless();
    radio = 1;
  }
  trimiteMail();
 }
  if (now.minute() == byte(EEPROM.read(addr)) && now.second() == 0){    //totul este ok
    pahar = 1;
      if(digitalRead(cup) == LOW && addr == 1){
          digitalWrite(dir, HIGH);
          stare = 2000;
          delay(600);
              while (stare > 800) {
                digitalWrite(dir, HIGH);
                stare = analogRead(box);
                delay(5);
                }
                digitalWrite(dir, LOW);                  
                cont2 = 0;
                pahar = 0;
                obs = 1;
                mail = 0;
                msgIaPill();
        }
         if(digitalRead(cup) == LOW && addr == 2){
          digitalWrite(viteza, HIGH);
              while (distrib < 875) {
                digitalWrite(viteza, HIGH);
                distrib = analogRead(cama);
                delay(5);
                }
                digitalWrite(viteza, LOW);                  
                cont2 = 0;
                pahar = 0;
                obs = 1;
                mail = 0;
                msgIaPill();
        }
        if(digitalRead(cup) == LOW && addr == 3){
          digitalWrite(viteza, HIGH);
              while (distrib < 2090) {
                digitalWrite(viteza, HIGH);
                distrib = analogRead(cama);
                delay(5);
                }
                digitalWrite(viteza, LOW);                  
                cont2 = 0;
                pahar = 0;
                obs = 1;
                mail = 0;
                msgIaPill();
        }
        if(digitalRead(cup) == LOW && addr == 4){
          digitalWrite(viteza, HIGH);
              while (distrib < 3190) {
                digitalWrite(viteza, HIGH);
                distrib = analogRead(cama);
                delay(5);
                }
                digitalWrite(viteza, LOW);                  
                cont2 = 0;
                pahar = 0;
                obs = 1;
                mail = 0;
                msgIaPill();
                delay(1000);
               while (distrib > 0){
                  digitalWrite(viteza , HIGH);
                  distrib = analogRead(cama);
                  delay(5);
                }
                //delay(1000);
                digitalWrite(viteza , LOW); 
        }                                               
    }
        if(digitalRead(cup) == HIGH && pahar == 1){                 //s-a facut timpul si "nu am paharul in aparat"
            cont1++;
        }
              if(cont1 == timp && pahar == 1){
                  msgNuCup(); 
                cont1 = 0;
                mail++;
              }
  if(digitalRead(cup) == LOW && pahar == 1 && obs == 0 && addr == 1){              //se executa dupa ce a fost pus paharul "ia pastilele"
     digitalWrite(dir, HIGH);
     stare = 2000;
     delay(600);
        while (stare > 800) {
            digitalWrite(dir, HIGH);
            stare = analogRead(box);
            delay(5);
        }
            digitalWrite(dir, LOW);
            cont1 = 0;
            pahar = 0;
            obs = 1;
            mail = 0;
            msgIaPill();
  }
   if(digitalRead(cup) == LOW && pahar == 1 && obs == 0 && addr == 2){
          digitalWrite(viteza, HIGH);
              while (distrib < 875) {
                digitalWrite(viteza, HIGH);
                distrib = analogRead(cama);
                delay(5);
                }
                digitalWrite(viteza, LOW);                  
                cont2 = 0;
                pahar = 0;
                obs = 1;
                mail = 0;
                msgIaPill();
        }
        if(digitalRead(cup) == LOW && pahar == 1 && obs == 0 && addr == 3){
          digitalWrite(viteza, HIGH);
              while (distrib < 2090) {
                digitalWrite(viteza, HIGH);
                distrib = analogRead(cama);
                delay(5);
                }
                digitalWrite(viteza, LOW);                  
                cont2 = 0;
                pahar = 0;
                obs = 1;
                mail = 0;
                msgIaPill();
        }
        if(digitalRead(cup) == LOW && pahar == 1 && obs == 0 && addr == 4){
          digitalWrite(viteza, HIGH);
              while (distrib < 3190) {
                digitalWrite(viteza, HIGH);
                distrib = analogRead(cama);
                delay(5);
                }
                digitalWrite(viteza, LOW);                  
                cont2 = 0;
                pahar = 0;
                obs = 1;
                mail = 0;
                msgIaPill();
                 delay(1000);
               while (distrib > 0){
                  digitalWrite(viteza , HIGH);
                  distrib = analogRead(cama);
                  delay(5);
                }
                //delay(1000);
                digitalWrite(viteza , LOW); 
        }
        if (digitalRead(cup) == LOW && pahar == 0 ){        //pahar ok dar nu a luat pastilele "te rog ia pastilele"
          cont2++;
        }
            if(cont2 == timp && obs == 1){
               msgRogPill(); 
            cont2 = 0;
            mail++;
            }
        if (digitalRead(cup) == HIGH && pahar == 0 && obs == 1){          //a ridicat paharul din aparat
          pahar = 1;  
        }
            if (digitalRead(cup) == LOW && pahar == 1 && obs == 1){       //a pus paharul si "multumesc" si next program
                msgThx();
             cont1 = 0;
             cont2 = 0;
             mail = 0;
             pahar = 0;
             obs = 0;
             addr++;  
            }
EEPROM.write(60, addr);                                                   // salveaza status in memorie
EEPROM.commit();

if(addr > 4){addr = 1;}                                                                 //reia programul

  if(digitalRead(pinOk) == LOW && stare >= 800) {                                         //load cartus
        digitalWrite(dir, HIGH);
          stare = 2000;
          delay(300);
              while (stare > 800) {
                digitalWrite(dir, HIGH);
                stare = analogRead(box);
                delay(5);
                }
                digitalWrite(dir, LOW);
                distrib = analogRead(cama);
                while (distrib > 0){
                  digitalWrite(viteza , HIGH);
                  distrib = analogRead(cama);
                  delay(5);
                }
                //delay(1000);
                digitalWrite(viteza , LOW);
  }
            load = 1;
            digitalWrite(dir, LOW);
            delay(10);
}

void sendCallback(SendStatus msg) {
      Serial.println(msg.info());
          if (msg.success()) {
                Serial.println("----------------");
          }
}
