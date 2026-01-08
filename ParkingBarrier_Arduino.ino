#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

// ================= LCD I2C =================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= SERVO =================
Servo myservo;

// ================= BLUETOOTH =================
SoftwareSerial bt(2, 3); // RX, TX (Arduino)

// ================= ULTRASONIC (IESIRE) =================
const int trigPin = 4;
const int echoPin = 5;

// ================= RFID (INTRARE) =================
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

// UID CARD VALID
byte validUID[4] = {0x9D, 0xF1, 0x2C, 0x1F};

// ================= VARIABILE =================
int totalLocuri = 4;
int locuriRamase = 4;

bool masinaIesireDetectata = false;

long tmeduration;
int distance;

// ======================================================
void setup() {
  Serial.begin(9600);
  bt.begin(9600);

  // Servo
  myservo.attach(6);
  myservo.write(0); // inchis la start

  // Ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Parcare Ready");
  delay(1500);

  // RFID
  SPI.begin();
  rfid.PCD_Init();

  Serial.println("Sistem pornit");
}

// ======================================================
void deschideBariera() {
  myservo.write(90);
  delay(3000);
  myservo.write(0);
}

// ======================================================
void loop() {

  // ================= BLUETOOTH =================
  if (bt.available()) {
    String cmd = bt.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();

    Serial.print("BT: ");
    Serial.println(cmd);

    if (cmd == "OPEN") {
      lcd.clear();
      lcd.print("BT OPEN");
      deschideBariera();
    }
  }

  // ================= AFISARE =================
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Locuri:");
  lcd.print(locuriRamase);

  // ======================================================
  // ================= INTRARE - RFID ====================
  // ======================================================
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

    bool valid = true;
    for (byte i = 0; i < 4; i++) {
      if (rfid.uid.uidByte[i] != validUID[i]) {
        valid = false;
      }
    }

    if (valid && locuriRamase > 0) {
      lcd.setCursor(0, 1);
      lcd.print("Acces permis");
      deschideBariera();
      locuriRamase--;
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Acces respins");
      delay(1500);
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // ======================================================
  // ================= IESIRE - ULTRASONIC ================
  // ======================================================
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  tmeduration = pulseIn(echoPin, HIGH, 30000);
  distance = (0.034 * tmeduration) / 2;

  if (distance > 0 && distance <= 10 && !masinaIesireDetectata) {
    lcd.setCursor(0, 1);
    lcd.print("Iesire...");
    deschideBariera();

    if (locuriRamase < totalLocuri) {
      locuriRamase++;
    }

    masinaIesireDetectata = true;
  }

  if (distance > 20) {
    masinaIesireDetectata = false;
  }

  delay(300);
}
