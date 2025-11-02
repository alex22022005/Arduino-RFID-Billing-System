#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#define buzzerPin 3
#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16 chars and 2 line display

byte tag1[4] = {0xFA, 0x7B, 0x16, 0xB0};  // Tag for milk
byte tag2[4] = {0x93, 0x4E, 0xEB, 0xA9};  // Tag for biscuit

float milkPrice = 10.00;  // Price for milk
float biscuitPrice = 40.00;  // Price for biscuit

int milkQuantity = 0;
int biscuitQuantity = 0;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();
  pinMode(buzzerPin, OUTPUT); // Set buzzer pin as output
  lcd.setCursor(0, 0);
  lcd.print("Loading...");
  delay(2000); // Wait for 2 seconds
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RFID Billing");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan RFID");
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.println("Card detected");
    digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
    delay(200); // Wait for a while
    digitalWrite(buzzerPin, LOW); // Turn off the buzzer

    if (checkTag(tag1)) {
      milkQuantity++;
    } else if (checkTag(tag2)) {
      biscuitQuantity++;
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Unknown tag");
      delay(1000);
      return; // Skip the rest of the loop if unknown tag
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Total: Rs");
    lcd.print((milkQuantity * milkPrice) + (biscuitQuantity * biscuitPrice), 2); // Print total amount with 2 decimal places
    lcd.setCursor(0, 1);
    lcd.print("Bis: ");
    lcd.print(milkQuantity);
    lcd.print(" Milk: ");
    lcd.print(biscuitQuantity);
   
    delay(2000);
  }
}

bool checkTag(byte tagToCompare[]) {
  for (int i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] != tagToCompare[i]) {
      return false;
    }
  }
  return true;
}