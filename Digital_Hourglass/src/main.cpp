#include <Arduino.h>

#define LONG_PRESS_TIME 1000 // distinguish between short action and long action
#define CLICK_INTERVAL 300   // threshold time to enable each led
#define BUZZER_BEEP_TIMES 2
#define SHORT_ACTION 0
#define LONG_ACTION 1

// pins
uint8_t buzzer_pin = 9;
uint8_t button_pin = 8;
uint8_t led_pin = 2; // start from pin 2 to 7

// global variables
uint32_t currentTime = 0;
uint32_t previousTime = 0;
uint16_t buttonPressTime = 0;
uint8_t buzzerBeepCount = 0;
bool actionMode = SHORT_ACTION;

// function prototypes
void mainProcess();

void setup()
{
  Serial.begin(115200);

  for (led_pin; led_pin <= 7; led_pin++)
  {
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);
  }

  pinMode(button_pin, INPUT_PULLUP);
  pinMode(buzzer_pin, OUTPUT);
}

void loop()
{
  mainProcess();
}

void mainProcess()
{
  currentTime = millis();
  if (digitalRead(button_pin) == LOW) // press button
  {
    while (digitalRead(button_pin) == LOW) // wait for button to be released
    {
    }

    buttonPressTime = millis() - currentTime;

    actionMode = (buttonPressTime > LONG_PRESS_TIME) ? LONG_ACTION : SHORT_ACTION;
    switch (actionMode)
    {
    case LONG_ACTION:
      for (led_pin; led_pin >= 2; led_pin--)
      {
        digitalWrite(led_pin, LOW);
        delay(500);
      }
      for (buzzerBeepCount = 0; buzzerBeepCount < BUZZER_BEEP_TIMES; buzzerBeepCount++)
      {
        digitalWrite(buzzer_pin, HIGH);
        delay(500);
        digitalWrite(buzzer_pin, LOW);
        delay(200);
      }
      break;

    case SHORT_ACTION:
      if (currentTime - previousTime > CLICK_INTERVAL)
      {
        previousTime = currentTime;
        digitalWrite(led_pin, HIGH);
        if (led_pin < 7)
        {
          led_pin++;
        }
      }
      break;

    default:
      break;
    }

    buttonPressTime = 0;
  }
}