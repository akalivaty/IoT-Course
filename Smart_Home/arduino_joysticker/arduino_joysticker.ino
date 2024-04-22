#include <Arduino.h>
#include <SoftwareSerial.h>

#define PHOTOR_THRESHOLE 400 // depends on the photoresistor
#define VOLUME_THRESHOLE 20

SoftwareSerial BTserial(2, 3); // SoftwareSerial(RX, TX) on arduino board

const uint8_t button1 = 6;
const uint8_t button2 = 7;
const uint8_t photoR = A0;
const uint8_t joystick_x = A1;
const uint8_t joystick_y = A2;
const uint8_t joystick_sw = 10;

const int bufferLength = 20;              // 定义缓存大小为10个字节
char serialBuffer[bufferLength] = {'\0'}; // 建立字符数组用于缓存
String message = "";
uint8_t bitMessage = 0;

void MainProcess();
void BTProcess();
void TestJoystick();

void setup()
{
  Serial.begin(115200);
  BTserial.begin(38400);

  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(photoR, INPUT);
  pinMode(joystick_x, INPUT);
  pinMode(joystick_y, INPUT);
  pinMode(joystick_sw, INPUT);
}

void loop()
{
  MainProcess();
  // BTProcess();
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

void BTProcess()
{
  if (BTserial.available())
  {
    for (int i = 0; i < bufferLength; i++)
    {
      serialBuffer[i] = '\0';
    }
    BTserial.readBytes(serialBuffer, bufferLength);
    Serial.write(serialBuffer);
    Serial.write('\n');
    if(atoi(serialBuffer) == 753)
    {
      BTserial.write(0xFF);
      BTserial.write('\n');
    }
  }
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