#include <Arduino.h>

#define PHOTOR_THRESHOLD 400

// pins
byte buzzer_pin = 7;
byte button_pin = 6;

// function prototypes
void mainProcess();

void setup()
{
  Serial.begin(115200);

  pinMode(buzzer_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
}

void loop()
{
  mainProcess();
  delay(1000);
}

void mainProcess()
{
  Serial.print("sensor: ");
  Serial.println(digitalRead(button_pin));

  if(digitalRead(button_pin) == LOW)  // press button
  {
    digitalWrite(buzzer_pin, HIGH);
    delay(500);
    digitalWrite(buzzer_pin, LOW);
  }
}