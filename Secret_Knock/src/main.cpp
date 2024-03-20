#include <Arduino.h>
#include <Servo.h>

Servo servo;

// pins
const uint8_t knockSensor = A0;  // Piezo sensor on pin 0.
const uint8_t programButton = 2; // If this is high we program a new code.
const uint8_t redLED = 4;        // Status LED
const uint8_t greenLED = 5;      // Status LED
const uint8_t servo_pin = 9;     // Servo motor on pin 9.

// global variables
const int threshold = 100;          // Minimum signal from the piezo to register as a knock
const int rejectValue = 25;         // If an individual knock is off by this percentage of a knock we don't unlock..
const int averageRejectValue = 15;  // If the average timing of the knocks is off by this percent we don't unlock.
const int knockFadeTime = 150;      // milliseconds we allow a knock to fade before we listen for another one. (Debounce timer.)
const uint16_t lockTurnTime = 1000; // milliseconds that we run the motor to get it to go a half turn.

const uint8_t maximumKnocks = 20; // Maximum number of knocks to listen for.
const int knockComplete = 1200;   // Longest time to wait for a knock before we assume that it's finished.

int secretCode[maximumKnocks] = {50, 25, 25, 50, 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Initial setup: "Shave and a Hair Cut, two bits."
uint32_t knockReadings[maximumKnocks] = {0};               // interval between knocks
int knockSensorValue = 0;                                  // Last reading of the knock sensor.
int programButtonPressed = false;                          // Flag so we remember the programming button setting at the end of the cycle.

uint8_t servoPos = 0;
int32_t startTime = 0;
int32_t now = 0;

// function prototypes
void mainProcess();
void listenToSecretKnock();
bool validateKnock();
void triggerDoorUnlock();

void setup()
{
  Serial.begin(115200);

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(programButton, INPUT);

  // initialize servo
  servo.attach(servo_pin);
  servo.write(0);
  delay(1000);
  servo.detach();

  digitalWrite(greenLED, HIGH);     // Green LED on, everything is go.
  Serial.println("Program start."); // but feel free to comment them out after it's working right.
}

void loop()
{
  mainProcess();
}

void mainProcess()
{
  // Listen for any knock at all.
  knockSensorValue = analogRead(knockSensor);

  if (digitalRead(programButton) == HIGH)
  {
    programButtonPressed = true;
    digitalWrite(redLED, HIGH);
  }
  else
  {
    programButtonPressed = false;
    digitalWrite(redLED, LOW);
  }

  if (knockSensorValue >= threshold) // knock is valid
  {
    listenToSecretKnock();
  }
}

// Records the timing of knocks.
void listenToSecretKnock()
{
  Serial.println("knock starting");

  uint8_t i = 0;
  // First lets reset the listening array.
  for (i = 0; i < maximumKnocks; i++)
  {
    knockReadings[i] = 0;
  }

  uint8_t currentKnockNumber = 0; // Incrementer for the array.
  startTime = millis();           // Reference for when this knock started.
  now = millis();

  digitalWrite(greenLED, LOW); // blink the LED for a bit as a visual indicator of the knock.
  if (programButtonPressed == true)
  {
    digitalWrite(redLED, LOW); // and the red one too if we're programming a new knock.
  }
  delay(knockFadeTime); // wait for this peak to fade before we listen to the next one.
  digitalWrite(greenLED, HIGH);
  if (programButtonPressed == true)
  {
    digitalWrite(redLED, HIGH);
  }
  do
  {
    // listen for the next knock or wait for it to timeout.
    knockSensorValue = analogRead(knockSensor);
    if (knockSensorValue >= threshold)
    {
      Serial.println("knock.");
      now = millis();
      knockReadings[currentKnockNumber] = now - startTime;
      currentKnockNumber++;

      startTime = now;
      digitalWrite(greenLED, LOW);
      if (programButtonPressed == true)
      {
        digitalWrite(redLED, LOW);
      }
      delay(knockFadeTime);
      digitalWrite(greenLED, HIGH);
      if (programButtonPressed == true)
      {
        digitalWrite(redLED, HIGH);
      }
    }
    now = millis();
  } while ((now - startTime < knockComplete) && (currentKnockNumber < maximumKnocks)); // timeout or run out of knocks?

  // we've got our knock recorded, lets see if it's valid
  if (programButtonPressed == false)
  {
    if (validateKnock() == true)
    {
      triggerDoorUnlock();
    }
    else
    {
      Serial.println("Secret knock failed.");
      digitalWrite(greenLED, LOW); // We didn't unlock, so blink the red LED as visual feedback.
      for (i = 0; i < 4; i++)
      {
        digitalWrite(redLED, HIGH);
        delay(100);
        digitalWrite(redLED, LOW);
        delay(100);
      }
      digitalWrite(greenLED, HIGH);
    }
  }
  else
  { // if we're in programming mode we still validate the lock, we just don't do anything with the lock
    validateKnock();
    // and we blink the green and red alternately to show that program is complete.
    Serial.println("New lock stored.");
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
    for (i = 0; i < 3; i++)
    {
      delay(100);
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, LOW);
      delay(100);
      digitalWrite(redLED, LOW);
      digitalWrite(greenLED, HIGH);
    }
  }
}

// Runs the motor (or whatever) to unlock the door.
void triggerDoorUnlock()
{
  Serial.println("Door unlocked!");

  // turn the motor
  servo.attach(servo_pin);
  for (servoPos = 0; servoPos < 90; servoPos += 1)
  {
    servo.write(servoPos);
    delay(15);
  }
  servo.detach();
  digitalWrite(greenLED, HIGH); // And the green LED too.

  delay(lockTurnTime); // Wait a bit.

  servo.attach(servo_pin);
  for (servoPos = 90; servoPos > 0; servoPos -= 1)
  {
    servo.write(servoPos);
    delay(15);
  }
  servo.detach();

  // Blink the green LED a few times for more visual feedback.
  for (uint8_t i = 0; i < 5; i++)
  {
    digitalWrite(greenLED, LOW);
    delay(100);
    digitalWrite(greenLED, HIGH);
    delay(100);
  }
}

// Sees if our knock matches the secret.
// returns true if it's a good knock, false if it's not.
// todo: break it into smaller functions for readability.
boolean validateKnock()
{
  int currentKnockCount = 0;
  int secretKnockCount = 0;
  uint32_t maxKnockInterval = 0; // to normalize the times with map later

  uint8_t i = 0;
  for (; i < maximumKnocks; i++)
  {
    if (knockReadings[i] > 0)
    {
      currentKnockCount++;
    }
    if (secretCode[i] > 0)
    {
      secretKnockCount++;
    }

    if (knockReadings[i] > maxKnockInterval)
    { // collect normalization data while we're looping.
      maxKnockInterval = knockReadings[i];
    }
  }

  // If we're recording a new knock, save the info and get out of here.
  if (programButtonPressed == true)
  {
    for (i = 0; i < maximumKnocks; i++)
    { // normalize the times
      secretCode[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    }
    // And flash the lights in the recorded pattern to let us know it's been programmed.
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, LOW);
    delay(1000);
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, HIGH);
    delay(50);
    for (i = 0; i < maximumKnocks; i++)
    {
      digitalWrite(greenLED, LOW);
      digitalWrite(redLED, LOW);
      // only turn it on if there's a delay
      if (secretCode[i] > 0)
      {
        delay(map(secretCode[i], 0, 100, 0, maxKnockInterval)); // Expand the time back out to what it was.  Roughly.
        digitalWrite(greenLED, HIGH);
        digitalWrite(redLED, HIGH);
      }
      delay(50);
    }
    return false; // We don't unlock the door when we are recording a new knock.
  }

  if (currentKnockCount != secretKnockCount)
  {
    return false;
  }

  /*  Now we compare the relative intervals of our knocks, not the absolute time between them.
      (ie: if you do the same pattern slow or fast it should still open the door.)
      This makes it less picky, which while making it less secure can also make it
      less of a pain to use if you're tempo is a little slow or fast.
  */
  uint32_t totaltimeDifferences = 0;
  uint32_t timeDiff = 0;
  for (i = 0; i < maximumKnocks; i++)
  { // Normalize the times
    knockReadings[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    timeDiff = abs(knockReadings[i] - secretCode[i]);
    if (timeDiff > rejectValue)
    { // Individual value too far out of whack
      return false;
    }
    totaltimeDifferences += timeDiff;
  }
  // It can also fail if the whole thing is too inaccurate.
  if (totaltimeDifferences / secretKnockCount > averageRejectValue)
  {
    return false;
  }

  return true;
}