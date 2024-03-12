#include <Arduino.h> // only for PlatformIO project
#include <Servo.h>

#define ENTRY_THRESHOLE 36 // depends on the photoresistor
#define EXIT_THRESHOLE 2   // depends on the photoresistor
#define OPEN 0             // for gateControl()
#define CLOSE 1            // for gateControl()
#define MAX_CAR 3

// objects
Servo servo;

// pins
byte ledR_pin = 12;
byte ledG_pin = 11;
byte ledB_pin = 10;
byte servo_pin = 8;
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

  pinMode(ledR_pin, OUTPUT);
  pinMode(ledG_pin, OUTPUT);
  pinMode(ledB_pin, OUTPUT);
  pinMode(photoR_entry, INPUT);
  pinMode(photoR_exit, INPUT);

  // initialize servo
  servo.attach(servo_pin);
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
  // a car is trying to get in
  if (gateState == CLOSE && analogRead(photoR_entry) < ENTRY_THRESHOLE && carNumber < MAX_CAR)
  {
    gateControl(OPEN);
    gateState = OPEN;
    while (gateState == OPEN) // wait for the car to pass
    {
      if (analogRead(photoR_exit) < EXIT_THRESHOLE)
      {
        gateControl(CLOSE);
        gateState = CLOSE;
        carNumber += 1;
        setColor();
      }
    }
  }

  // a car is trying to get out
  if (gateState == CLOSE && analogRead(photoR_exit) < EXIT_THRESHOLE && carNumber > 0)
  {
    gateControl(OPEN);
    gateState = OPEN;
    while (gateState == OPEN) // wait for the car to pass
    {
      if (analogRead(photoR_entry) < ENTRY_THRESHOLE)
      {
        gateControl(CLOSE);
        gateState = CLOSE;
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
    servo.attach(servo_pin);
    for (servoPos = 0; servoPos < 90; servoPos += 1)
    {
      servo.write(servoPos);
      delay(15);
    }
    servo.detach();
  }
  else
  {
    servo.attach(servo_pin);
    for (servoPos = 90; servoPos > 0; servoPos -= 1)
    {
      servo.write(servoPos);
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
  analogWrite(ledR_pin, r);
  analogWrite(ledG_pin, g);
  analogWrite(ledB_pin, b);
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