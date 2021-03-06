#include <Arduino.h>
#include <Servo.h>

static constexpr uint8_t const MOTOR_PIN = 2;
static constexpr uint8_t const BUTTON_PIN = 9;
static constexpr uint8_t const SERVO_PIN = 3;

enum Event
{
  INVALID = 0,
  NONE,
  BUTTON_DOWN,
  BUTTON_UP,
};

class FSM;

class State
{
  public:
  virtual void execute(FSM* fsm)
  {
  }
  virtual void onEnter(FSM* fsm)
  {
  }
  virtual void onExit(FSM* fsm)
  {
  }
  virtual void event(FSM* fsm, Event event)
  {
  }
};

class OnState : public State
{
  void execute(FSM* fsm);
  void event(FSM* fsm, Event event);
};
static OnState onState;

class OffState : public State
{
  void execute(FSM* fsm);
  void event(FSM* fsm, Event event);
};
static OffState offState;

class BlinkState : public State
{
  public:
  BlinkState() : lastChange(0), value(HIGH)
  {
  }
  void execute(FSM* fsm);
  void event(FSM* fsm, Event event);
  void onEnter(FSM* fsm)
  {
    lastChange = millis();
    value = HIGH;
  }

  private:
  unsigned long lastChange;
  uint8_t value;
};
static BlinkState blinkState;

class FSM
{
  public:
  FSM(State* firstState) : curState(firstState)
  {
  }
  ~FSM()
  {
  }

  void execute()
  {
    curState->execute(this);
  }

  void event(Event event)
  {
    curState->event(this, event);
  }

  void next(State* nextState)
  {
    curState->onExit(this);
    curState = nextState;
    curState->onEnter(this);
  }

  private:
  FSM();
  State* curState;
};

void OnState::execute(FSM* fsm)
{
  digitalWrite(MOTOR_PIN, HIGH);
}

void OnState::event(FSM* fsm, Event event)
{
  if (event == BUTTON_DOWN)
  {
    fsm->next(&blinkState);
  }
}

void OffState::execute(FSM* fsm)
{
  digitalWrite(MOTOR_PIN, LOW);
}

void OffState::event(FSM* fsm, Event event)
{
  if (event == BUTTON_DOWN)
  {
    fsm->next(&onState);
  }
}

void BlinkState::execute(FSM* fsm)
{
  auto curTime = millis();
  if (curTime - lastChange < 500)
  {
    return;
  }
  lastChange = curTime;
  value = (value == LOW ? HIGH : LOW);
  digitalWrite(MOTOR_PIN, value);
}

void BlinkState::event(FSM* fsm, Event event)
{
  if (event == BUTTON_DOWN)
  {
    fsm->next(&offState);
  }
}

class Button
{
  public:
  Button(uint8_t pin) : pin(pin)
  {
  }

  void begin()
  {
    lastDebounceTime = millis();
    lastButtonState = digitalRead(BUTTON_PIN);
  }

  Event getEvent()
  {
    Event retVal = NONE;
    uint8_t reading = digitalRead(BUTTON_PIN);
    if (reading != lastButtonState)
    {
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > 50)
    {
      if (reading != buttonState)
      {
        buttonState = reading;
        if (buttonState == HIGH)
        {
          retVal = BUTTON_UP;
        }
        else
        {
          retVal = BUTTON_DOWN;
        }
      }
    }
    lastButtonState = reading;
    return retVal;
  }

  private:
  uint8_t pin;
  unsigned long lastDebounceTime;
  uint8_t lastButtonState;
  uint8_t buttonState;
};

static FSM fsm(&offState);
static Button button(BUTTON_PIN);
static Servo head;

void setup()
{
  Serial.begin(115200);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  button.begin();
  head.attach(SERVO_PIN);
}

void loop()
{
  static int headPos = 0;
  static int increment = 1;
  static auto lastWrite = millis();
  fsm.execute();
  fsm.event(button.getEvent());

  auto curTime = millis();
  if (curTime - lastWrite < 8)
  {
    return;
  }
  head.write(headPos);
  headPos = headPos + increment;
  if (headPos == 156)
  {
    increment = -1;
  }
  else if (headPos == -1)
  {
    increment = 1;
  }
  lastWrite = curTime;
}