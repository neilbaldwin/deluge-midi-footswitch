
//
// Deluge MIDI Footswitch Controller
// Neil Baldwin June 2020
//

#include <MIDI.h>
#include <AceButton.h>
using namespace ace_button;

// Define input pins for switches
const int inputPin1 = 2;
const int inputPin2 = 3;
const int inputPin3 = 4;
const int inputPin4 = 5;
// Define output pins for LEDs
const int ledPin1 = 23;
const int ledPin2 = 22;
const int ledPin3 = 21;
const int ledPin4 = 20;
const int ledPin = 13;
// Define MIDI note numbers for Deluge commands
const int delugeNote1 = 116;
const int delugeNote2 = 117;
const int delugeNote3 = 118;
const int delugeNote4 = 119;
const int delugeNote5 = 120;
const int delugeNote6 = 121;
const int delugeNote7 = 122;
const int delugeNote8 = 123;
const int delugeNote9 = 124;
const int delugeNote10 = 125;
const int delugeNote11 = 126;
const int delugeNote12 = 127; // invalid as button 4 long press = setup mode
const int delugeVelocity = 100;

// Running modes
// modeNormal is normal 4 button operation
// modeSetup is setup mode to change MIDI channel with button 2 and 3, hold 4 to exit
// modeEnterSetup and modeExitSetup just transition modes that flash the LEDs to
//    let user know they are entering/exiting setup
const int modeNormal = 0;
const int modeEnterSetup = 1;
const int modeSetup = 2;
const int modeExitSetup = 3;

// Structure that holds input pin and corresponding LED/outut pin
struct Info
{
  const uint8_t inputPin;
  const uint8_t ledPin;
};

// Array of input/LED structs for iterration
Info INFOS[4] = {
    {inputPin1, ledPin1},
    {inputPin2, ledPin2},
    {inputPin3, ledPin3},
    {inputPin4, ledPin4},
};

// Array of 4 AceButton instances
AceButton buttons[4];
// One handler takes care of all buttons inputs
void buttonHandler(AceButton *, uint8_t, uint8_t);
// declarations for functions to avoid compiler errors, actuall definition at end of source
void ledOn(uint8_t led);
void ledOff(uint8_t led);
void flashLeds();
void showChannelLeds();

// Arrays of Deluge MIDI note numbers
// We have three sets of MIDI keys depending on whether button
// is clicked, double-clicked or held
// Button 4 long press enters setup menu
int delugeKeysClicked[] = {delugeNote1, delugeNote2, delugeNote3, delugeNote4};
int delugeKeysDoubleClicked[] = {delugeNote5, delugeNote6, delugeNote7, delugeNote8};
int delugeKeysLongPressed[] = {delugeNote9, delugeNote10, delugeNote11, delugeNote12};

// Default MIDI channel is 16 (0x0F)
int delugeChannel = 0x0F;

// Initial running mode
int runningMode = modeExitSetup;

// Create instance of MIDI output device on Serial1
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

void setup()
{
  delay(1000);
  Serial.begin(9600);

  // Set pin modes for input s and outputs as AceButton does not do this
  for (int p = 0; p < 4; p++)
  {
    pinMode(INFOS[p].inputPin, INPUT_PULLUP);
    pinMode(INFOS[p].ledPin, OUTPUT);
    // also initialise buttons in AceButton instances
    buttons[p].init(INFOS[p].inputPin, HIGH, p);
  }

  // ButtonConfig setup stuff
  ButtonConfig *buttonConfig = ButtonConfig::getSystemButtonConfig();
  // Set AceButton handler (one for all buttons)
  buttonConfig->setEventHandler(buttonHandler);
  // Add the features you want to detect
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  // Set some settings
  buttonConfig->setLongPressDelay(1000); // how long to press before long press is registered
  buttonConfig->setDoubleClickDelay(50); // time between two clicks to register as double click
  // Set suppression modes to cancel handler after certain events to
  //   prevent double triggers
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

  // Initialise MIDI output
  MIDI.begin();
}

void buttonHandler(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
  // Delay between NOTE ON and NOTE OFF - might not need this at all
  const uint8_t midiDelay = 10;
  // get the button id (0 to 3) that triggered handler
  uint8_t id = button->getId();
  // get corresponding led pin
  uint8_t ledPin = INFOS[id].ledPin;

  if (runningMode == modeNormal)
  {
    switch (eventType)
    {
    // Normal mode, keys send midi note (and delayed midi note off)
    case AceButton::kEventClicked:
    case AceButton::kEventReleased:
      ledOn(ledPin);
      delugeNoteOn(delugeKeysClicked[id]);
      delay(midiDelay);
      delugeNoteOff(delugeKeysClicked[id]);
      delay(midiDelay);
      ledOff(ledPin);
      break;

    case AceButton::kEventDoubleClicked:
      ledOn(ledPin);
      delugeNoteOn(delugeKeysDoubleClicked[id]);
      delay(midiDelay);
      delugeNoteOff(delugeKeysDoubleClicked[id]);
      delay(midiDelay);
      ledOff(ledPin);
      break;

    case AceButton::kEventLongPressed:
      // Buttons ID 0..2 (1 to 3) can send MIDI note on long press
      // Button ID 3 (4) long press initiates setup mode
      if (id == 3)
      {
        runningMode = modeEnterSetup;
      }
      else
      {
        ledOn(ledPin);
        delugeNoteOn(delugeKeysLongPressed[id]);
        delay(midiDelay);
        delugeNoteOff(delugeKeysLongPressed[id]);
        delay(midiDelay);
        ledOff(ledPin);
        break;
      }
    }
  }
  else if (runningMode == modeSetup)
  {
    // In setup mode, button 2/3 increments/decrements midi channel
    // Long press button 4 to exit setup
    switch (eventType)
    {
    case AceButton::kEventClicked:
    case AceButton::kEventReleased:
      switch (id)
      {
      case 1:
        delugeChannel = (delugeChannel + 1) & 0x0F;
        break;
      case 2:
        delugeChannel = (delugeChannel - 1) & 0x0F;
        break;
      }
      break;
    // Long press button 4 (ID 3) to exit setup
    case AceButton::kEventLongPressed:
      switch (id)
      {
      case 3:
        runningMode = modeExitSetup;
        break;
      }
      break;
    }
  }
}

//
// Main Loop
//
void loop()
{
  // check buttons
  for (int b = 0; b < 4; b++)
  {
    buttons[b].check();
  }

  // Simple switch case to handle transitions between normal and setup mode
  switch (runningMode)
  {
  // Normal = buttons send MIDI notes
  case modeNormal:
    break;

  // Entering setup, extra mode just to flash LEDs indicating setup mode
  case modeEnterSetup:
    flashLeds();
    runningMode = modeSetup;
    break;

  case modeSetup:
    showChannelLeds();
    break;

  // exiting setup mode, flash LEDs to indicate to user
  case modeExitSetup:
    flashLeds();
    runningMode = modeNormal;
    break;
  }
}

// Read bits 0..3 of MIDI channel and display on LEDs
void showChannelLeds()
{
  for (int b = 0; b < 4; b++)
  {
    if (bitRead(delugeChannel, b))
    {
      ledOn(INFOS[b].ledPin);
    }
    else
    {
      ledOff(INFOS[b].ledPin);
    }
  }
}

// turn single LED on
void ledOn(uint8_t led)
{
  digitalWrite(led, HIGH);
}

// turn single LED off
void ledOff(uint8_t led)
{
  digitalWrite(led, LOW);
}

// turn all LEDs off
void allLedsOff()
{
  for (int b = 0; b < 4; b++)
  {
    ledOff(INFOS[b].ledPin);
  }
}

// Indicator for mode change, flash all LEDs several times
void flashLeds()
{
  int flashDelay = 25;
  int flashes = 4;
  for (int f = 0; f < flashes; f++)
  {
    for (int b = 0; b < 4; b++)
    {
      ledOn(INFOS[b].ledPin);
      delay(flashDelay);
      ledOff(INFOS[b].ledPin);
      delay(flashDelay);
    }
  }
}

// Send MIDI note on
void delugeNoteOn(int delugeButton)
{
  MIDI.sendNoteOn(delugeButton, delugeVelocity, delugeChannel + 1);
  Serial.println(delugeButton);
}

// Send MIDI note off
void delugeNoteOff(int delugeButton)
{
  MIDI.sendNoteOff(delugeButton, 0, delugeChannel + 1);
}
