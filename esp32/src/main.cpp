#include <Arduino.h>
#include <RCSwitch.h>
#include <ezButton.h>

#define ARRAY_LENGTH(array) (sizeof((array)) / sizeof((array)[0]))

#define DEBUG true

#define DISCONNECT_DELAY 1000

/*

R0 -> NC=oALN, NO=oBLN, C=R4/NC
R1 -> NC=oARN, NO=oBRN, C=R5/NC
R2 -> NC=oALP, NO=oBLP, C=R6/NCprintf
R3 -> NC=oARP, NO=oBRP, C=R7/NC
R4 -> NC=R0/C, NO=,     C=iLN
R5 -> NC=R1/C, NO=,     C=iRN
R6 -> NC=R2/C, NO=,     C=iLP
R7 -> NC=R3/C, NO=,     C=iRP

### Switch to A

// Disconnect inputs
R4,R5 -> ON
R7,R7 -> ON

delay(100)

// Switch outputs
R0,R1,R2,R3 -> OFF

delay(100)

// Reconnect inputs
R4,R5 -> OFF
R6,R7 -> OFF

### Switch to B

// Disconnect inputs
R4,R5 -> ON
R6,R7 -> ON

delay(100)

// Switch outputs
R0,R1,R2,R3 -> ON

delay(100)

// Reconnect inputs
R4,R5 -> OFF
R6,R7 -> OFF
*/

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
const int relays[8] = {13, 14, 27, 26, 25, 33, 32, 15};

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

    // Set relays status

    digitalWrite(relays[4], HIGH); // Disconnect inputs
    digitalWrite(relays[5], HIGH);
    digitalWrite(relays[6], HIGH);
    digitalWrite(relays[7], HIGH);
    delay(DISCONNECT_DELAY);
    digitalWrite(relays[0], inputSelected == INPUT_A ? LOW : HIGH);
    digitalWrite(relays[1], inputSelected == INPUT_A ? LOW : HIGH);
    digitalWrite(relays[2], inputSelected == INPUT_A ? LOW : HIGH);
    digitalWrite(relays[3], inputSelected == INPUT_A ? LOW : HIGH);
    delay(DISCONNECT_DELAY);
    digitalWrite(relays[4], LOW); // Reconnect inputs
    digitalWrite(relays[5], LOW);
    digitalWrite(relays[6], LOW);
    digitalWrite(relays[7], LOW);
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