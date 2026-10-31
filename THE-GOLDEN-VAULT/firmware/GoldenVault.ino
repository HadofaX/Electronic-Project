#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// ================= RFID =================

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);


// ================= SERVO =================

Servo vaultServo;

const int SERVO_PIN = 6;

const int CLOSED_ANGLE = 0;
const int OPEN_ANGLE = 90;


// ================= LEDs =======

const int RED_LED    = 2;
const int BLUE_LED   = 3;
const int GREEN_LED  = 4;
const int YELLOW_LED = 5;


// ================= Buttons =====

const int RED_BUTTON    = 8;
const int BLUE_BUTTON   = A0;
const int GREEN_BUTTON  = A1;
const int YELLOW_BUTTON = A2;


// ================= Arrays =====

const int leds[] = {
  RED_LED,
  BLUE_LED,
  GREEN_LED,
  YELLOW_LED
};

const int buttons[] = {
  RED_BUTTON,
  BLUE_BUTTON,
  GREEN_BUTTON,
  YELLOW_BUTTON
};


// ================= Color Sequence =====

const int sequence[] = {
  0, 1, 2, 3
};

const int sequenceLength = 4;


// ================= SETUP =====

void setup() {

  Serial.begin(9600);

  // RFID
  SPI.begin();
  rfid.PCD_Init();

  // Servo
  vaultServo.attach(SERVO_PIN);
  vaultServo.write(CLOSED_ANGLE);

  // LEDs
  for (int i = 0; i < 4; i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }

  // Buttons
  for (int i = 0; i < 4; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
  }

  Serial.println();
  Serial.println("================================");
  Serial.println("       THE GOLDEN VAULT");
  Serial.println("================================");
  Serial.println("System Locked...");
  Serial.println();
  Serial.println("STEP 1: Scan RFID card");
}


// ================= MAIN LOOP =====

void loop() {

  // Wait Print
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // print UID
  Serial.print("UID: ");

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      Serial.print(":");
    }
  }

  Serial.println();


  if (isCorrectRFID()) {

    Serial.println("================================");
    Serial.println("✅ RFID ACCEPTED!");
    Serial.println("Stage 1 Completed!");
    Serial.println("================================");

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    delay(1000);

    bool solved = colorPuzzle();

    if (solved) {

      Serial.println();
      Serial.println("================================");
      Serial.println("🎉 COLOR PUZZLE SOLVED!");
      Serial.println("Stage 2 Completed!");
      Serial.println("================================");

      delay(1000);

      // Open Vault
      openVault();

      // توقف النظام بعد فتح الخزنة
      while (true) {
        delay(1000);
      }
    }

  } else {

    Serial.println("❌ WRONG RFID!");
    Serial.println("Access Denied!");

    errorEffect();

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    delay(1000);
  }
}


// ================= RFID CHECK ====

bool isCorrectRFID() {

  if (rfid.uid.size != 4) {
    return false;
  }

  return (
    rfid.uid.uidByte[0] == 0x33 &&
    rfid.uid.uidByte[1] == 0xC1 &&
    rfid.uid.uidByte[2] == 0x3F &&
    rfid.uid.uidByte[3] == 0x21
  );
}


// ================= COLOR PUZZLE ===

bool colorPuzzle() {

  Serial.println();
  Serial.println("STEP 2: COLOR PUZZLE");
  Serial.println();

  delay(1000);

  showSequence();

  Serial.println();
  Serial.println("Repeat the sequence!");

  while (true) {

    for (int i = 0; i < sequenceLength; i++) {

      int pressedColor = waitForButton();

      Serial.print("Pressed: ");
      Serial.println(colorName(pressedColor));

      // error
      if (pressedColor != sequence[i]) {

        Serial.println("❌ WRONG!");
        Serial.println("Restarting puzzle...");

        errorEffect();

        delay(1000);

        showSequence();

        Serial.println();
        Serial.println("Try again!");

        break;
      }

      successPress(pressedColor);

      // Last BUTTON
      if (i == sequenceLength - 1) {
        return true;
      }
    }
  }
}


/// ================= SHOW SEQUENCE ===

void showSequence() {

  Serial.println("MEMORIZE:");

  for (int i = 0; i < sequenceLength; i++) {

    int color = sequence[i];

    Serial.println(colorName(color));

    digitalWrite(leds[color], HIGH);

    delay(700);

    digitalWrite(leds[color], LOW);

    delay(300);
  }
}


/// ================= WAIT FOR BUTTON ===

int waitForButton() {

  while (true) {

    for (int i = 0; i < 4; i++) {

      if (digitalRead(buttons[i]) == LOW) {

        delay(50);

        /// wait BUTTON
        while (digitalRead(buttons[i]) == LOW) {
          delay(10);
        }

        return i;
      }
    }
  }
}


// ================= COLOR NAME ===

String colorName(int color) {

  switch (color) {

    case 0:
      return "RED";

    case 1:
      return "BLUE";

    case 2:
      return "GREEN";

    case 3:
      return "YELLOW";
  }

  return "UNKNOWN";
}


// ================= CORRECT PRESS ===

void successPress(int color) {

  digitalWrite(leds[color], HIGH);

  delay(200);

  digitalWrite(leds[color], LOW);
}


/// ================= ERROR EFFECT ===

void errorEffect() {

  for (int i = 0; i < 3; i++) {

    for (int j = 0; j < 4; j++) {
      digitalWrite(leds[j], HIGH);
    }

    delay(150);

    for (int j = 0; j < 4; j++) {
      digitalWrite(leds[j], LOW);
    }

    delay(150);
  }
}


/// ================= VAULT OPEN =================

void openVault() {

  Serial.println();
  Serial.println("================================");
  Serial.println("🔓 ACCESS GRANTED!");
  Serial.println("OPENING GOLDEN VAULT...");
  Serial.println("================================");

  /// servo open
  vaultServo.write(OPEN_ANGLE);

  delay(1000);

  /// party
  for (int i = 0; i < 3; i++) {

    for (int j = 0; j < 4; j++) {
      digitalWrite(leds[j], HIGH);
    }

    delay(250);

    for (int j = 0; j < 4; j++) {
      digitalWrite(leds[j], LOW);
    }

    delay(250);
  }

  Serial.println("🪙 GOLD VAULT OPEN!");
}
