#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo myservo;

SoftwareSerial bt(2, 3);

const int trigPin = 4;
const int echoPin = 5;

#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

byte validUID[4] = {0x9D, 0xF1, 0x2C, 0x1F};

int totalLocuri = 4;
int locuriRamase = 4;

bool masinaIesireDetectata = false;

long tmeduration;
int distance;

void setup() {
  Serial.begin(9600);
  bt.begin(9600);

  myservo.attach(6);
  myservo.write(0); 

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Parcare Ready");
  delay(1500);

  SPI.begin();
  rfid.PCD_Init();

  Serial.println("Sistem pornit");
}

void deschideBariera() {
  myservo.write(90);
  delay(3000);
  myservo.write(0);
}

void loop() {

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

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Locuri:");
  lcd.print(locuriRamase);

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

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  tmeduration = pulseIn(echoPin, HIGH, 30000);
  distance = (0.034 * tmeduration) / 2;

  Serial.print("Distanta ultrasonic: ");
  Serial.print(distance);
  Serial.println(" cm");


  if (distance > 0 && distance <= 10 && !masinaIesireDetectata) {
    lcd.setCursor(0, 1);
    lcd.print("Iesire...");
    deschideBariera();

    if (locuriRamase < totalLocuri) {
      locuriRamase++;
    }

    masinaIesireDetectata = true;
  }

  if (distance == 0) {
    masinaIesireDetectata = false;
  }

  delay(300);
}
