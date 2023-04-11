#include <Arduino.h>
#include <RCSwitch.h>
#include <ezButton.h>

#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

#define DEBUG true

// State
#define INPUT_A 0
#define INPUT_B 1
uint previousInputSelected = -1;
uint inputSelected = -1;

// Input selection button
#define PUSH_BUTTON_PIN 22
ezButton button(PUSH_BUTTON_PIN);

// RF 433Mhz remote controller
#define RC_SWITCH_PIN 34
#define BUTTON1_ON_CODE 83029
#define BUTTON1_OFF_CODE 83028
#define BUTTON2_ON_CODE 86101
#define BUTTON2_OFF_CODE 86100
#define BUTTON3_ON_CODE 70741
#define BUTTON3_OFF_CODE 70740
#define BUTTON4_ON_CODE 21589
#define BUTTON4_OFF_CODE 21588
RCSwitch mySwitch = RCSwitch();

// Relays
const int relays[2] = {27, 26};

// Led
#define LED1_RED_PIN 19
#define LED1_BLUE_PIN 21

void handleInputSelection(uint selection)
{
  previousInputSelected = inputSelected;
  inputSelected = selection;

  if (previousInputSelected != inputSelected)
  {
    Serial.printf("Input selection changed: %u -> %u\n", previousInputSelected, inputSelected);

    // Set relays status
    if (inputSelected == INPUT_A)
    {
      digitalWrite(relays[0], LOW);
      digitalWrite(relays[1], HIGH);
    }
    else
    {
      digitalWrite(relays[0], HIGH);
      digitalWrite(relays[1], LOW);
    }

    // Set leds status
    if (inputSelected == INPUT_A)
    {
      digitalWrite(LED1_RED_PIN, HIGH);
      digitalWrite(LED1_BLUE_PIN, LOW); // ON
    }
    else
    {
      digitalWrite(LED1_RED_PIN, LOW); // ON
      digitalWrite(LED1_BLUE_PIN, HIGH);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  // Input selection button
  button.setDebounceTime(50);

  // Remote controller
  mySwitch.enableReceive(digitalPinToInterrupt(RC_SWITCH_PIN));

  // Relays
  for (int i = 0; i < ARRAY_LENGTH(relays); i++)
  {
    pinMode(relays[i], OUTPUT);
    digitalWrite(relays[i], HIGH);
  }

  // Led
  pinMode(LED1_RED_PIN, OUTPUT);
  pinMode(LED1_BLUE_PIN, OUTPUT);

  // Init output
  handleInputSelection(INPUT_A);

  Serial.print("Initialized.");
}

void loop()
{

  button.loop();

  // Button
  if (button.isPressed())
  {
    Serial.println("Selection button pressed");
    handleInputSelection(inputSelected == INPUT_A ? INPUT_B : INPUT_A);
  }

  if (button.isReleased())
  {
    Serial.println("Selection button released");
  }

  // RF switch
  if (mySwitch.available())
  {

    const unsigned long receivedCode = mySwitch.getReceivedValue();

    if (DEBUG)
    {
      const unsigned int bit = mySwitch.getReceivedBitlength();
      Serial.print("Received ");
      Serial.print(receivedCode);
      Serial.print(" / ");
      Serial.print(bit);
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println(mySwitch.getReceivedProtocol());
    }

    if (receivedCode == BUTTON1_ON_CODE)
    {
      handleInputSelection(INPUT_A);
    }
    else if (receivedCode == BUTTON1_OFF_CODE)
    {
      handleInputSelection(INPUT_B);
    }
    else
    {
      Serial.printf("Don't know how to handle RF code %u\n", receivedCode);
    }

    mySwitch.resetAvailable();
  }
}