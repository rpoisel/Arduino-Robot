#include <Arduino.h>

static constexpr uint8_t const MOTOR_PIN = 2;
static constexpr uint8_t const BUTTON_PIN = 9;

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
    fsm->next(&offState);
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

void setup()
{
  Serial.begin(115200);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  button.begin();
}

void loop()
{
  fsm.execute();
  fsm.event(button.getEvent());
}