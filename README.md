# Arduino-RFID-Billing-System

A simple Arduino-based billing system that uses an MFRC522 RFID reader to scan items. Scan predefined tags to add them to a cart, and the total cost is instantly calculated and displayed on a 16x2 I2C LCD. The project includes a buzzer for audio feedback on each successful scan.

---

## Features

* **RFID Scanning**: Uses the MFRC522 module to read RFID tag UIDs.
* **Item Tracking**: Counts quantities for multiple distinct items (e.g., "Milk" and "Biscuit").
* **Live Totaling**: Instantly calculates and displays the total price on an LCD.
* **User Feedback**: A 16x2 I2C LCD shows item counts and totals.
* **Audio Feedback**: A buzzer beeps on every successful card scan.

---

## Hardware Required

* Arduino (Uno, Nano, or any compatible board)
* MFRC522 RFID Reader Module
* 16x2 I2C LCD Display
* Active or Passive Buzzer
* Jumper Wires
* Breadboard (optional)
* RFID Tags/Cards

---

## Software & Libraries

* [Arduino IDE](https://www.arduino.cc/en/software)
* [MFRC522 Library](https://github.com/miguelbalboa/rfid) by Miguel Balboa
* [LiquidCrystal_I2C Library](https://github.com/johnrickman/LiquidCrystal_I2C) by John Rickman

You can install these libraries directly from the Arduino IDE Library Manager.

---

## Wiring

| Component | Arduino Pin |
| :--- | :--- |
| **MFRC522 (SPI)** | |
| SDA (SS) | Pin 10 (or as defined in `SS_PIN`) |
| SCK | Pin 13 |
| MOSI | Pin 11 |
| MISO | Pin 12 |
| RST | Pin 9 (or as defined in `RST_PIN`) |
| 3.3V | 3.3V |
| GND | GND |
| **I2C LCD** | |
| SCL | A5 |
| SDA | A4 |
| VCC | 5V |
| GND | GND |
| **Buzzer** | |
| + (Positive) | Pin 3 (or as defined in `buzzerPin`) |
| - (Negative) | GND |

---

## How to Use

1.  **Find Your Tag UIDs**: Before you can use this code, you need the unique IDs (UIDs) of your specific RFID tags.
    * In the Arduino IDE, go to `File > Examples > MFRC522 > DumpInfo`.
    * Upload this sketch to your Arduino.
    * Open the Serial Monitor (Baud: 9600).
    * Scan each of your tags. The Serial Monitor will display its UID, which looks something like `FA 7B 16 B0`.

2.  **Update the Code**: In the `RFID_Billing_System.ino` sketch, replace the UIDs in these lines with the UIDs you just found:
    ```cpp
    byte tag1[4] = {0xFA, 0x7B, 0x16, 0xB0};  // Tag for milk
    byte tag2[4] = {0x93, 0x4E, 0xEB, 0xA9};  // Tag for biscuit
    ```
    *Note: If your tag has a different UID length (e.g., 7 bytes), you must update the array size and the `checkTag` function accordingly.*

3.  **Upload the Main Sketch**: After updating the UIDs, upload the main project sketch to your Arduino.

4.  **Run**: Power on the system. The LCD will show "Scan RFID". Scan your tags to add items, and the display will update with the item counts and the total price.

---

## Project Code

Here is the complete source code for the project.

```cpp
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

// --- Pin Definitions ---
#define buzzerPin 3
#define SS_PIN 10
#define RST_PIN 9

// --- Module Initialization ---
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address (0x27 is common)

// --- Item Database ---
// !! REPLACE THESE WITH YOUR OWN TAG UIDs !!
byte tag1[4] = {0xFA, 0x7B, 0x16, 0xB0};  // Tag for milk
byte tag2[4] = {0x93, 0x4E, 0xEB, 0xA9};  // Tag for biscuit

float milkPrice = 10.00;    // Price for milk
float biscuitPrice = 40.00; // Price for biscuit

// --- Global Variables ---
int milkQuantity = 0;
int biscuitQuantity = 0;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();
  pinMode(buzzerPin, OUTPUT); // Set buzzer pin as output

  // Startup LCD Sequence
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
  // Look for new cards
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.println("Card detected");
    
    // Beep buzzer
    digitalWrite(buzzerPin, HIGH);
    delay(200);
    digitalWrite(buzzerPin, LOW);

    // Check which tag was scanned
    if (checkTag(tag1)) {
      milkQuantity++;
      Serial.println("Milk added.");
    } else if (checkTag(tag2)) {
      biscuitQuantity++;
      Serial.println("Biscuit added.");
    } else {
      Serial.println("Unknown tag.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Unknown tag");
      delay(1000);
      rfid.PICC_HaltA(); // Halt the tag
      rfid.PCD_StopCrypto1(); // Stop crypto
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Scan RFID");
      return; // Skip the rest of the loop
    }

    // Update LCD Display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Total: Rs");
    // Print total amount with 2 decimal places
    lcd.print((milkQuantity * milkPrice) + (biscuitQuantity * biscuitPrice), 2);
    
    lcd.setCursor(0, 1);
    lcd.print("Bis: ");
    lcd.print(biscuitQuantity); // Flipped variable in original code
    lcd.print(" Milk: ");
    lcd.print(milkQuantity); // Flipped variable in original code
    
    rfid.PICC_HaltA(); // Halt the tag to allow new tags to be read
    rfid.PCD_StopCrypto1(); // Stop crypto
    delay(1000); // Short delay before next scan
  }
}

/**
 * Helper function to compare the scanned tag UID with a known tag UID.
 */
bool checkTag(byte tagToCompare[]) {
  // Check if UID sizes match (basic check)
  if (rfid.uid.size != 4) {
    return false;
  }
  
  for (int i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] != tagToCompare[i]) {
      return false;
    }
  }
  return true;
}
```
