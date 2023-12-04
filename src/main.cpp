//  SPDX-FileCopyrightText: 2023 Robert Grizzell
//  SPDX-License-Identifier: MIT
#include <Arduino.h>
#include <Wire.h>

#include <queue>

#define I2C_ADDR 0x55
#define SDA_PIN 2
#define SCL_PIN 10
#define LED_PIN 9
#define INT_PIN 46

using namespace std;

void i2cReceive(int bytesRead);
void i2cTransmit();
void readKeyMatrix();
void processKeyStates();
bool keyReleased(int colIndex, int rowIndex);
bool keyPressed(int colIndex, int rowIndex);
bool isAssignedKey(int colIndex, int rowIndex);
void addKeyEvent(char key);
void setBacklight(bool state);

uint8_t rowPins[] = {0, 3, 19, 12, 18, 6, 7};
uint8_t colPins[] = {1, 4, 5, 11, 13};
const int rowCount = sizeof(rowPins);
const int colCount = sizeof(colPins);

bool keys[colCount][rowCount];
bool lastValue[colCount][rowCount];
bool changedValue[colCount][rowCount];
char keyboard[colCount][rowCount];
char keyboardSymbol[colCount][rowCount];

bool backlightState = false;
queue<char> eventBuffer;
uint8_t eventsRequested = 1;

void setup() {
  keyboard[0][0] = 'q';
  keyboard[0][1] = 'w';
  keyboard[0][2] = '\0';  // Symbol
  keyboard[0][3] = 'a';
  keyboard[0][4] = '\0';  // Alt
  keyboard[0][5] = ' ';
  keyboard[0][6] = '\0';  // Mic

  keyboard[1][0] = 'e';
  keyboard[1][1] = 's';
  keyboard[1][2] = 'd';
  keyboard[1][3] = 'p';
  keyboard[1][4] = 'x';
  keyboard[1][5] = 'z';
  keyboard[1][6] = '\0';  // Left Shift

  keyboard[2][0] = 'r';
  keyboard[2][1] = 'g';
  keyboard[2][2] = 't';
  keyboard[2][3] = '\0';  // Right Shift
  keyboard[2][4] = 'v';
  keyboard[2][5] = 'c';
  keyboard[2][6] = 'f';

  keyboard[3][0] = 'u';
  keyboard[3][1] = 'h';
  keyboard[3][2] = 'y';
  keyboard[3][3] = (char)0x0a;  // Enter
  keyboard[3][4] = 'b';
  keyboard[3][5] = 'n';
  keyboard[3][6] = 'j';

  keyboard[4][0] = 'o';
  keyboard[4][1] = 'l';
  keyboard[4][2] = 'i';
  keyboard[4][3] = (char)0x08;  // Backspace
  keyboard[4][4] = '$';
  keyboard[4][5] = 'm';
  keyboard[4][6] = 'k';

  keyboardSymbol[0][0] = '#';
  keyboardSymbol[0][1] = '1';
  keyboardSymbol[0][2] = '\0';  // Symbol
  keyboardSymbol[0][3] = '*';
  keyboardSymbol[0][4] = '\0';  // Alt
  keyboardSymbol[0][5] = ' ';
  keyboardSymbol[0][6] = '0';

  keyboardSymbol[1][0] = '2';
  keyboardSymbol[1][1] = '4';
  keyboardSymbol[1][2] = '5';
  keyboardSymbol[1][3] = '@';
  keyboardSymbol[1][4] = '8';
  keyboardSymbol[1][5] = '7';
  keyboardSymbol[1][6] = '\0';  // Left Shift

  keyboardSymbol[2][0] = '3';
  keyboardSymbol[2][1] = '/';
  keyboardSymbol[2][2] = '(';
  keyboardSymbol[2][3] = '\0';  // Right Shift
  keyboardSymbol[2][4] = '?';
  keyboardSymbol[2][5] = '9';
  keyboardSymbol[2][6] = '6';

  keyboardSymbol[3][0] = '_';
  keyboardSymbol[3][1] = ':';
  keyboardSymbol[3][2] = ')';
  keyboardSymbol[3][3] = (char)0x0a;  // Enter
  keyboardSymbol[3][4] = '!';
  keyboardSymbol[3][5] = ',';
  keyboardSymbol[3][6] = ';';

  keyboardSymbol[4][0] = '+';
  keyboardSymbol[4][1] = '"';
  keyboardSymbol[4][2] = '-';
  keyboardSymbol[4][3] = (char)0x7f;  // Delete
  keyboardSymbol[4][4] = '\0';
  keyboardSymbol[4][5] = '.';
  keyboardSymbol[4][6] = '\'';

  // Set PIN modes for keyboard matrix and backlight
  for (int x = 0; x < rowCount; x++) {
    pinMode(rowPins[x], INPUT);
  }
  for (int x = 0; x < colCount; x++) {
    pinMode(colPins[x], INPUT_PULLUP);
  }
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, backlightState);

  // Start Serial and I2C communication
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Wire.onReceive(i2cReceive);
  Wire.onRequest(i2cTransmit);
  Wire.begin((uint8_t)I2C_ADDR, SDA_PIN, SCL_PIN, 0);
  Serial.print("Started I2C interface on address: ");
  Serial.println(I2C_ADDR);
}

void loop() {
  readKeyMatrix();
  processKeyStates();
}

/**
 * Receives and parses data from the host device.
 */
void i2cReceive(int bytesRead) {
  if (0 < bytesRead) {
    byte address = Wire.read();
    Serial.printf("REC[%x]: %x -> ", bytesRead, address);

    byte data[bytesRead - 1];
    for (byte i = 0; i < (bytesRead - 1); i++) {
      byte x = Wire.read();
      Serial.print(x);
      data[i] = x;
    }
    Serial.println();

    // switch (address) {
    //   case 0x0:
    //     if (0 < sizeof(data)) {
    //       eventsRequested = (uint8_t)data[0];
    //     } else {
    //       eventsRequested = 1;
    //     }
    //     break;
    //   case 0x1:
    //     key = eventBuffer.size();
    //     break;
    //   case 0x10:
    //     setBacklight(!backlightState);
    //     break;
    //   case 0x11:
    //     setBacklight(true);
    //     break;
    //   case 0x12:
    //     setBacklight(false);
    //     break;
    //   default:
    //     Serial.print("UNKNOWN: ");
    //     Serial.println(address);
    // }
  }
}

/**
 * Builds and transmits a response to the host device.
 */
void i2cTransmit() {
  // TODO: Request multiple keypresses at one time.
  byte response[eventsRequested];
  for (byte i = 0; i < eventsRequested; i++) {
    if (0 < eventBuffer.size()) {
      response[i] = (byte)eventBuffer.front();
      eventBuffer.pop();
    } else {
      response[i] = '\0';
    }
  }
  Wire.write(response, sizeof(response));
  eventsRequested = 1;  // Reset
}

/**
 * Reads the keyboard pin states and stores them for processing.
 */
void readKeyMatrix() {
  for (int colIndex = 0; colIndex < colCount; colIndex++) {
    uint8_t curCol = colPins[colIndex];
    pinMode(curCol, OUTPUT);
    digitalWrite(curCol, LOW);

    for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {
      uint8_t rowCol = rowPins[rowIndex];
      pinMode(rowCol, INPUT_PULLUP);
      delay(1);  // Not fast enough to switch input/output modes

      bool buttonPressed = (digitalRead(rowCol) == LOW);
      keys[colIndex][rowIndex] = buttonPressed;
      if ((lastValue[colIndex][rowIndex] != buttonPressed)) {
        changedValue[colIndex][rowIndex] = true;
      } else {
        changedValue[colIndex][rowIndex] = false;
      }
      lastValue[colIndex][rowIndex] = buttonPressed;
      pinMode(rowCol, INPUT);
    }
    pinMode(curCol, INPUT);
  }
}

/**
 * Processes the keyboard state and adds key events to the queue.
 */
void processKeyStates() {
  // Hot keys
  if (keyPressed(0, 4) && keyReleased(3, 4)) {
    Serial.println("HOTKEY: Alt + B");
    setBacklight(!backlightState);
  } else if (keyPressed(0, 4) && keyReleased(4, 1)) {
    Serial.println("HOTKEY: Alt + L");
    addKeyEvent((char)0x0c);

    // Regular keys
  } else {
    for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {
      for (int colIndex = 0; colIndex < colCount; colIndex++) {
        if (keyReleased(colIndex, rowIndex) &&
            isAssignedKey(colIndex, rowIndex)) {
          char key;
          // Symbol Modifier
          if (keyPressed(0, 2)) {
            addKeyEvent(keyboardSymbol[colIndex][rowIndex]);
            // Shift Modifier
          } else if ((keyPressed(1, 6) || keyPressed(2, 3)) &&
                     isLowerCase((uint8_t)keyboard[colIndex][rowIndex])) {
            addKeyEvent((char)((uint8_t)keyboard[colIndex][rowIndex] - 32));
          } else {
            addKeyEvent(keyboard[colIndex][rowIndex]);
          }
        }
      }
    }
  }
}

/**
 * Returns true if the key was pressed and released.
 */
bool keyReleased(int colIndex, int rowIndex) {
  return changedValue[colIndex][rowIndex] && keys[colIndex][rowIndex];
}

/**
 * Returns true if key was pressed but not released.
 */
bool keyPressed(int colIndex, int rowIndex) { return keys[colIndex][rowIndex]; }

/**
 * Checks if the key is a printing character.
 */
bool isAssignedKey(int colIndex, int rowIndex) {
  return keyboardSymbol[colIndex][rowIndex] != '\0' ||
         keyboard[colIndex][rowIndex] != '\0';
}

/**
 * Adds a key character event to the buffer.
 */
void addKeyEvent(char key) {
  Serial.print("EVENT: ");
  Serial.println(key);
  eventBuffer.push(key);
}

/**
 * Turns the keyboard backlight on or off.
 */
void setBacklight(bool state) {
  digitalWrite(LED_PIN, state);
  backlightState = state;
  Serial.print("LIGHT: ");
  Serial.println(state);
}
