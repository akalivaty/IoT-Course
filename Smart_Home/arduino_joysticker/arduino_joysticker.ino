#include <Arduino.h>


const uint8_t button1 = 6;
const uint8_t button2 = 7;
const uint8_t joystick_x = A1;
const uint8_t joystick_y = A2;
const uint8_t joystick_sw = 10;

uint8_t bitMessage = 0;

void MainProcess();
void BTProcess();
void TestJoystick();

void setup()
{
  Serial.begin(115200);

  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(joystick_x, INPUT);
  pinMode(joystick_y, INPUT);
  pinMode(joystick_sw, INPUT);
}

void loop()
{
  MainProcess();
}

void MainProcess()
{
  bitMessage = 0;
  if (digitalRead(button1) == LOW)
  {
    bitMessage += 64;
  }
  if (digitalRead(button2) == LOW)
  {
    bitMessage += 32;
  }
  JoystickMapping();
  if(bitMessage != 0)
  {
    Serial.println(bitMessage);
  }
  delay(300);
}

void JoystickMapping()
{
  int x = analogRead(joystick_x);
  int y = analogRead(joystick_y);
  int sw = digitalRead(joystick_sw);

  // up, down, left, right, switch
  //  16,      8,    4,     2,      1
  if(x > 550)
    bitMessage += 2;
  if(x < 450)
    bitMessage += 4;
  if(y < 450)
    bitMessage += 16;
  if(y > 550)
    bitMessage += 8;
  if(sw == LOW)
    bitMessage += 1;
}

void TestJoystick()
{
  int x = analogRead(joystick_x);
  int y = analogRead(joystick_y);
  int sw = digitalRead(joystick_sw);
  Serial.print("x: ");
  Serial.print(x);
  Serial.print(" y: ");
  Serial.print(y);
  Serial.print(" sw: ");
  Serial.println(sw);
}