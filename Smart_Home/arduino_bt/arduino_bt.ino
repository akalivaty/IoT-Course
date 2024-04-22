#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Servo.h>


// objects
Servo servo;
SoftwareSerial BTserial(2, 3);  // SoftwareSerial(RX, TX) on arduino board

// pins
uint8_t led_pin = 7;
uint8_t servo_pin = 8;

const int bufferLength = 20;
char serialBuffer[bufferLength] = { '\0' };
byte servoPos = 0;
bool gateState = false;

void MainProcess();
void BTProcess();

void setup() {
  Serial.begin(115200);
  BTserial.begin(38400);

  pinMode(led_pin, OUTPUT);

  // initialize servo
  servo.attach(servo_pin);
  servo.write(0);
  delay(1000);
  servo.detach();
}

void loop() {
  BTProcess();
}

void MainProcess() {
}

void BTProcess() {
  if (BTserial.available()) {
    for (int i = 0; i < bufferLength; i++) {
      serialBuffer[i] = '\0';
    }
    BTserial.readBytes(serialBuffer, bufferLength);

    Serial.print("command: ");
    Serial.println(atoi(serialBuffer));
    switch (atoi(serialBuffer)) {
      case 13:  // turn on light
        digitalWrite(led_pin, HIGH);
        break;

      case 17:  // turn off light
        digitalWrite(led_pin, LOW);
        break;

      case 23:  // open gate
        if (gateState == false) {
          gateState = true;
          servo.attach(servo_pin);
          for (servoPos = 0; servoPos < 90; servoPos += 1) {
            servo.write(servoPos);
            delay(15);
          }
          servo.detach();
        }
        break;

      case 29:  // close gate
        if (gateState == true) {
          gateState = false;
          servo.attach(servo_pin);
          for (servoPos = 90; servoPos > 0; servoPos -= 1) {
            servo.write(servoPos);
            delay(15);
          }
          servo.detach();
        }
        break;
    }
  }
}