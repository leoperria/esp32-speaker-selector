#include <Arduino.h>
#include <RCSwitch.h>
#include <ezButton.h>

#define ARRAY_LENGTH(array) (sizeof((array)) / sizeof((array)[0]))

#define DEBUG true

// Relays
const int relays[1] = {15};

// State
#define INPUT_A 0
#define INPUT_B 1
unsigned int previousInputSelected = -1;
unsigned int inputSelected = -1;

// RF 433Mhz remote controller
#define RC_SWITCH_PIN 2
#define BUTTON1_ON_CODE 83029
#define BUTTON1_OFF_CODE 83028
#define BUTTON2_ON_CODE 86101
#define BUTTON2_OFF_CODE 86100
#define BUTTON3_ON_CODE 70741
#define BUTTON3_OFF_CODE 70740
#define BUTTON4_ON_CODE 21589
#define BUTTON4_OFF_CODE 21588
RCSwitch mySwitch = RCSwitch();

// Input selection button
#define PUSH_BUTTON_PIN 5
ezButton button(PUSH_BUTTON_PIN);

// Led
#define LED1_RED_PIN 3
#define LED1_BLUE_PIN 4

void handleInputSelection(unsigned int selection)
{

	previousInputSelected = inputSelected;
	inputSelected = selection;

	if (previousInputSelected != inputSelected)
	{
		char buffer[80];
		sprintf(buffer, "Input selection changed: %u -> %u\n", previousInputSelected, inputSelected);
		Serial.print(buffer);

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
		digitalWrite(relays[0], inputSelected == INPUT_A ? LOW : HIGH);
	}
}

// the setup routine runs once when you press reset:
void setup()
{
	// Remote controller
	mySwitch.enableReceive(digitalPinToInterrupt(RC_SWITCH_PIN));

	// Input selection button
	pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);
	button.setDebounceTime(50);

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

	Serial.println("Initialized");
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
		else if (receivedCode == BUTTON2_ON_CODE)
		{
			handleInputSelection(INPUT_B);
		}
		else
		{
			char buffer[80];
			sprintf(buffer, "Don't know how to handle RF code %lu\n", receivedCode);
			Serial.print(buffer);
		}

		mySwitch.resetAvailable();
	}
}