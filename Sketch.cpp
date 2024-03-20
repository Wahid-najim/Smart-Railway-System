#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Initialize LCD

// Pin definitions
const int servoPin = 6; // Servo for ultrasonic sensor action
const int trigPin = 4; // Ultrasonic sensor trigger pin
const int echoPin = 3; // Ultrasonic sensor echo pin
const int servoPin1 = 7; // Additional servo 1 for RFID action
const int servoPin2 = 7; 
const int RST_PIN = 9; 
const int SS_PIN = 10; 

// Objects
Servo servo; 
Servo servo1; 
Servo servo2; 
MFRC522 mfrc522(SS_PIN, RST_PIN);


long duration;
int distance;
int safeDistance = 10; 

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Attach servos
  servo.attach(servoPin);
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  servo.write(0); // Initialize servo to 0 degrees
  servo1.write(0); // Initialize servo1 to 0 degrees
  servo2.write(0); // Initialize servo2 to 0 degrees

  // Initialize RFID reader
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID Reader Initialized");

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Smart Railway ");
}

void loop() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // Display distance on serial monitor
  Serial.print("Distance: ");
  Serial.println(distance);

  // Check if distance is within safe range
  if (distance > safeDistance) {
    servo.write(90); // Move servo to 90 degrees
    Serial.println("Barrier Open");
    Serial.print(distance);
  } else {
    servo.write(0); // Return servo to 0 degrees
    Serial.println("Barrier Closed");
  }
  delay(500);

  // RFID card detection and validation
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return; // Exit if no new card is present
  }

  // Display UID of the detected RFID tag
  Serial.print("UID tag: ");
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();

  // Check if the RFID tag matches the authorized tag
  content.toUpperCase();
  if (content.substring(1) == "53 8B A2 1A") {
    Serial.println("Access Granted");
    lcd.setCursor(0, 1);
    lcd.print("Access Granted");
  } else {
    // Move additional servos if RFID does not match
    servo1.write(90);
    servo2.write(90);
    Serial.println("Access Denied");
    lcd.setCursor(0, 1);
    lcd.print("Access Denied");
    delay(2000); // Delay to display message before next read
  }
}
