#include <Arduino.h>
#include <Servo.h>

#define ENTRY_THRESHOLE 36
#define EXIT_THRESHOLE 2
#define OPEN 0
#define CLOSE 1

// objects
Servo servo;

// pins
byte ledR = 12;
byte ledG = 11;
byte ledB = 10;
byte servoPin = 8;
byte photoR_entry = A0;
byte photoR_exit = A1;

// global variables
byte carNumber = 0;
byte servoPos = 0;
bool gateState = CLOSE;

// function prototypes
void mainProcess();
void gateControl(bool);
void setColor();
void analogColor(byte, byte, byte);
void logPhotoRState();

void setup()
{
  Serial.begin(115200);

  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  pinMode(photoR_entry, INPUT);
  pinMode(photoR_exit, INPUT);

  servo.attach(servoPin);
  servo.write(0);
  delay(1000);
  servo.detach();
}

void loop()
{
  logPhotoRState();
  mainProcess();
  delay(1000);
}

void mainProcess()
{
  if (gateState == CLOSE && analogRead(photoR_entry) < ENTRY_THRESHOLE && carNumber < 3)
  {
    gateControl(OPEN);
    gateState = OPEN;
    while (gateState == OPEN)
    {
      if (analogRead(photoR_exit) < EXIT_THRESHOLE)
      {
        gateControl(CLOSE);
        gateState = !gateState;
        carNumber += 1;
        setColor();
      }
    }
  }

  if (gateState == CLOSE && analogRead(photoR_exit) < EXIT_THRESHOLE && carNumber > 0)
  {
    gateControl(OPEN);
    gateState = OPEN;
    while (gateState == OPEN)
    {
      if (analogRead(photoR_entry) < ENTRY_THRESHOLE)
      {
        gateControl(CLOSE);
        gateState = !gateState;
        carNumber -= 1;
        setColor();
      }
    }
  }
}

void gateControl(bool command)
{
  if (command == OPEN)
  {
    servo.attach(servoPin);
    for (int pos = 0; pos < 90; pos += 1)
    {
      servo.write(pos);
      delay(15);
    }
    servo.detach();
  }
  else
  {
    servo.attach(servoPin);
    for (int pos = 90; pos > 0; pos -= 1)
    {
      servo.write(pos);
      delay(15);
    }
    servo.detach();
  }
}

void setColor()
{
  switch (carNumber)
  {
  case 1:
    analogColor(19, 242, 7);
    break;
  case 2:
    analogColor(219, 242, 7);
    break;
  case 3:
    analogColor(245, 15, 15);
    break;
  default:
    analogColor(0, 0, 0);
  }
}

void analogColor(byte r, byte g, byte b)
{
  analogWrite(ledR, r);
  analogWrite(ledG, g);
  analogWrite(ledB, b);
}

void logPhotoRState()
{
  Serial.print("entry: ");
  Serial.print(analogRead(photoR_entry));
  Serial.print("\texit: ");
  Serial.print(analogRead(photoR_exit));
  Serial.print("\tcar: ");
  Serial.println(carNumber);
}