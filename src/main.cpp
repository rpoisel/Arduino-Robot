#include <Arduino.h>

static constexpr uint8_t const MOTOR_PIN = 2;
static constexpr uint8_t const BUTTON_PIN = 9;

void setup()
{
    pinMode(MOTOR_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop()
{
    digitalWrite(MOTOR_PIN, !digitalRead(BUTTON_PIN));
}