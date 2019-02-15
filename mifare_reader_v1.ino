/*
 * Mifare reader for checking UID and version of transpoder (classic/desfire etc).
 * 
 * Return on display and ser.mon. UID in HEX, and version of transpoder.
 * Written for selfedu, and as quick project for controling if randomUID are enabled(in highsecured ACS).
 * Used generic RFID MFRC522 modul, Arduino UNO, and display 1602 with I2C modul.
 * 
 * Known bugs: 
 * some strings (PICC_GetTypeName) they have more 16 char, so bleeding behind first line LCD, 
 * and first byte on second line (UID) on LCD havent 0.
 * SerialPrints are OK.
 * 
 * Pinouts:
 * rfid modul RC522 - supply 3Vmax !
 * ------
 * SDA-10
 * RST-9
 * SCK-13
 * MISO-12
 * MOSI-11
 * 
 * LCD 1602
 * -----
 * SCL-A5
 * SDA-A4
 * 
 * BEEP
 * -----
 * PWM7
 * 
 * OK1VBR (cl)2019
 */

#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
const int piskle = 7;  


void pipni()                        // Beep
  {
  digitalWrite(piskle, HIGH);
  delay(50);
  digitalWrite(piskle, LOW);
  }

void setup() 
  {
  pinMode(piskle, OUTPUT);
  Serial.begin(9600);
  SPI.begin();                    // Init SPI bus
  mfrc522.PCD_Init();             // Init MFRC522 
  lcd.begin();
  pipni();
  lcd.noBacklight();
  delay(200);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("MIFARE ctecka");
  lcd.setCursor(0,1); 
  lcd.print("OK1VBR ver.0.1");
  //lcd.display();
  }
  
void printHex(byte *buffer, byte bufferSize) // Process 
  {
  for (byte i = 0; i < bufferSize; i++) 
    {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    }
  }

void loop() 
{
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  readRFID();
  delay(100);
}

void readRFID() 
{
//Show version transpoder on serial and LCD
  lcd.clear();
  Serial.print("\n-----------------------------");  
  Serial.print("\nVerze: ");
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
  lcd.setCursor(0,0);
  lcd.print(mfrc522.PICC_GetTypeName(piccType)); //known bug-some strings are longest 16 pos
  
//Show UID on serial monitor and LCD
  Serial.print("UID:");
  String content= "";
  lcd.setCursor(0,1);
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
     lcd.print(String(mfrc522.uid.uidByte[i], HEX));
     pipni();   //beep after show uidByte
  }

/*
 * // For print in a DEC
  void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) 
    {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
    }
    }
*/


  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

}
