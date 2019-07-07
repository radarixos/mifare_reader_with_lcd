/* NFC reader with color display and bluetooth link (serial echo) to android phone 
 * selfedu project for arduino nano, under GNU/GPL
 * 
 * version 0.5, demo, 03/2019.
 * Arduino NANO - packet version
 * SPI disp, I2C PN532, BT HC04, keyboard via A0
 * 
 */
 
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <SoftwareSerial.h>

#define TFT_CS        10  // CS for TFT
#define TFT_RST        9  // Reset for TFT
#define TFT_DC         8  // DC(A0) for TFT
#define TFT_BACKLIGHT  6  // Display backlight pin
#define RX 2              // serial data RX modul HC04/06
#define TX 3              // serial data TX modul HC04/06
const int piskle = 7;     // beep (if UID was readed)

// default HARDWARE SPI pins Uno/Nano: 
// SPI MOSI = pin 11 (sda)
// SPI MISO = pin 12
// SPI SCLK = pin 13 (sck)

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
PN532_I2C pn532i2c(Wire);         // vytvoření instance I2C komunikace pro PN532 adr 0x24
PN532 nfc(pn532i2c);              // vytvoření instance PN532 NFC modulu
SoftwareSerial bluetooth(TX, RX); // default pair PIN 1234

void pipni()    {
  digitalWrite(piskle, HIGH);
  delay(50);
  digitalWrite(piskle, LOW);
  }

void setup() {
  Serial.begin(9600); 
  bluetooth.begin(9600);
  nfc.begin();
  pinMode(piskle, OUTPUT); 
  pinMode(TFT_BACKLIGHT, OUTPUT);
  //tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.initR(INITR_GREENTAB);        // Init ST7735S chip, greentab
  tft.setRotation(3);
  welcomeScreen();
  // uložení verze desky pro kontrolu jejího připojení
  uint32_t versiondata = nfc.getFirmwareVersion();
  // kontrola načtené verze NFC modulu
  if (!versiondata) {
    Serial.println("Nebyl nalezen PN53x modul!");
    bluetooth.println("Nebyl nalezen PN53x modul!");
    // zastavení programu
    while (1);
    }
  else {
    // vytištění informací o připojeném modulu
    Serial.print("Nalezen modul PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
    bluetooth.print("Nalezen modul PN5"); bluetooth.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print("Firmware verze "); Serial.print((versiondata >> 16) & 0xFF, DEC);
    bluetooth.print("Firmware modulu NFC - verze "); bluetooth.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
    bluetooth.print('.'); bluetooth.println((versiondata >> 8) & 0xFF, DEC);
    }
    
  // nastavení maximálního počtu pokusů o čtení NFC tagu,
  // 0xFF odpovídá cca jedné sekundě
  nfc.setPassiveActivationRetries(0x10);
  // konfigurace NFC modulu pro čtení tagů
  nfc.SAMConfig();
  // vytištění informace o začátku čtení
  Serial.println("PN532 modul nastaven, prilozte tag...");
  pipni();
}

void welcomeScreen() {
  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight TFT on
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(3,50);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_RED);
  tft.print("NFC ctecka"); bluetooth.println("NFC reader");
  tft.setCursor(3,70);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Radarixos, ver. 0.5");bluetooth.println("Radarixos, ver. 0.5");
  tft.setCursor(3,90);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print("Bluetooth serial"); bluetooth.println("Bluetooth - serial link seems OK!");
}

void printHex(byte *buffer, byte bufferSize)
  {
  for (byte i = 0; i < bufferSize; i++) 
    {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    }
  }

void cudliky() {
  String text;
  int analogData = analogRead(0);
  if (analogData < 50) text = "BUDIZ  ";                        //do zeme 0R
  if (analogData > 150 && analogData < 250) text = "DOLU   ";    //213
  if (analogData > 650 && analogData < 750) text = "NAHORU ";    //695
  if (analogData > 950 && analogData < 1100) text = "NIC    ";   //1023
  tft.fillRect(3,85,150,17,ST77XX_BLUE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5,90);
  tft.setTextSize(1);
  tft.print(text); tft.print(analogData);
  Serial.println(text); bluetooth.println(text);
  Serial.println(analogData);
}

void loop() {
  //cteni klavesnice na A0
  cudliky();
        
  //modry podklad a text "Nactena data:"
  tft.fillRect(3,5,150,17,ST77XX_BLUE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5,10);
  tft.setTextSize(1);
  tft.print("Nactena data:");

  // vytvoření proměnných, do kterých se budou ukládat výsledky čtení
  boolean uspech;                          // úspěšné čtení
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; // unikátní ID tagu
  uint8_t uidLength;                       // délka ID tagu

  // zahájení čtení tagů v okolí, výsledek čtení se uloží do nastavených proměnných
  uspech = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

  // pokud je čtení úspěšné, vypíšeme získané informace
  if (uspech) {
    Serial.println("Nacten NFC:"); bluetooth.println("Nacten NFC: ");
    pipni();
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(1);
    
    Serial.print("UID delka: "); 
    Serial.print(uidLength, DEC); 
    Serial.println(" bytu");
    tft.setCursor(5,50);
    tft.setTextColor(ST77XX_GREEN);
    tft.print("UID delka: "); 
    tft.setTextColor(ST77XX_WHITE); 
    tft.setCursor(5,60);
    tft.print(uidLength, DEC); 
    tft.print(" bytu");
    
    Serial.print("UID hodnoty: ");
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(5,25);
    tft.setTextSize(1);
    tft.print("UID v hex: "); bluetooth.println("UID v hex:");
    tft.setCursor(5,35);
   
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(" 0x"); Serial.print(uid[i], HEX);
      tft.setTextColor(ST77XX_WHITE);
      tft.print(uid[i] < 0x10 ? " 0" : " ");              //zapis s mezerama mezi bytama
      tft.print(uid[i], HEX);                             //zapis s mezerama mezi bytama
      bluetooth.print(uid[i] < 0x10 ? " 0" : " ");        //zapis s mezerama mezi bytama
      bluetooth.print(uid[i], HEX);                       //zapis s mezerama mezi bytama
     }
    Serial.println(""); bluetooth.println("");            //odradkovani na serialu a bluetooth
   }
  else {
    // vytištění informace o vypršení času čtení
    Serial.println("zadny tag.");
    }
}
