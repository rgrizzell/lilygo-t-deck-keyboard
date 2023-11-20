//  SPDX-FileCopyrightText: 2023 Robert Grizzell
//  SPDX-License-Identifier: MIT
#include <Arduino.h>
#include <Queue.h>
#include <Wire.h>

#define I2C_ADDR 0x55
#define SDA 2
#define SCL 10
#define LITE 9
#define INT 46

uint8_t rows[] = {0, 3, 19, 12, 18, 6, 7};
const int rowCount = sizeof(rows) / sizeof(rows[0]);
uint8_t cols[] = {1, 4, 5, 11, 13};
const int colCount = sizeof(cols) / sizeof(cols[0]);

bool keys[colCount][rowCount];
bool lastValue[colCount][rowCount];
bool changedValue[colCount][rowCount];
char keyboard[colCount][rowCount];
char keyboardSymbol[colCount][rowCount];

bool symbolSelected;
bool backlightState = false;
Queue i2cBuffer;

void onRequest();
void onReceive(int bytesRead);
void readMatrix();
bool keyPressed(int colIndex, int rowIndex);
bool keyActive(int colIndex, int rowIndex);
bool isPrintableKey(int colIndex, int rowIndex);
void printMatrix();
void setBacklight(bool state);

void setup() {
  keyboard[0][0] = 'q';
  keyboard[0][1] = 'w';
  keyboard[0][2] = '\0';  // Symbol
  keyboard[0][3] = 'a';
  keyboard[0][4] = '\0';  // At
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
  keyboard[3][3] = '\0';  // Enter
  keyboard[3][4] = 'b';
  keyboard[3][5] = 'n';
  keyboard[3][6] = 'j';

  keyboard[4][0] = 'o';
  keyboard[4][1] = 'l';
  keyboard[4][2] = 'i';
  keyboard[4][3] = '\0';  // Backspace
  keyboard[4][4] = '$';
  keyboard[4][5] = 'm';
  keyboard[4][6] = 'k';

  keyboardSymbol[0][0] = '#';
  keyboardSymbol[0][1] = '1';
  keyboardSymbol[0][2] = '\0';
  keyboardSymbol[0][3] = '*';
  keyboardSymbol[0][4] = '\0';
  keyboardSymbol[0][5] = '\0';
  keyboardSymbol[0][6] = '0';

  keyboardSymbol[1][0] = '2';
  keyboardSymbol[1][1] = '4';
  keyboardSymbol[1][2] = '5';
  keyboardSymbol[1][3] = '@';
  keyboardSymbol[1][4] = '8';
  keyboardSymbol[1][5] = '7';
  keyboardSymbol[1][6] = '\0';

  keyboardSymbol[2][0] = '3';
  keyboardSymbol[2][1] = '/';
  keyboardSymbol[2][2] = '(';
  keyboardSymbol[2][3] = '\0';
  keyboardSymbol[2][4] = '?';
  keyboardSymbol[2][5] = '9';
  keyboardSymbol[2][6] = '6';

  keyboardSymbol[3][0] = '_';
  keyboardSymbol[3][1] = ':';
  keyboardSymbol[3][2] = ')';
  keyboardSymbol[3][3] = '\0';
  keyboardSymbol[3][4] = '!';
  keyboardSymbol[3][5] = ',';
  keyboardSymbol[3][6] = ';';

  keyboardSymbol[4][0] = '+';
  keyboardSymbol[4][1] = '"';
  keyboardSymbol[4][2] = '-';
  keyboardSymbol[4][3] = '\0';
  keyboardSymbol[4][4] = '\0';
  keyboardSymbol[4][5] = '.';
  keyboardSymbol[4][6] = '\'';

  symbolSelected = false;

  // Set PIN modes for keyboard matrix and backlight
  for (int x = 0; x < rowCount; x++) {
    pinMode(rows[x], INPUT);
  }
  for (int x = 0; x < colCount; x++) {
    pinMode(cols[x], INPUT_PULLUP);
  }
  pinMode(LITE, OUTPUT);
  digitalWrite(LITE, backlightState);

  // Start Serial and I2C communication
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Wire.onRequest(onRequest);
  Wire.onReceive(onReceive);
  Wire.begin((uint8_t)I2C_ADDR, SDA, SCL, 0);
  Serial.print("Started I2C interface on address: ");
  Serial.println(I2C_ADDR);
}

void loop() {
  readMatrix();
  printMatrix();

  // Process keypresses
  if (keyPressed(3, 3)) {
    Serial.println("");
    i2cBuffer.push((char)0x0D);
  }
  if (keyPressed(4, 3)) {
    Serial.println("backspace");
    i2cBuffer.push((char)0x08);
  }
  if (keyActive(0, 4) && keyPressed(3, 4)) {
    Serial.println("Alt+B");
    setBacklight(!backlightState);
  }
  if (keyActive(0, 4) && keyPressed(4, 1)) {
    Serial.println("Alt+L");
    i2cBuffer.push((char)0x0C);
  }
}

void onRequest() {
  char key;
  if (i2cBuffer.size() > 0) {
    key = (char)i2cBuffer.pop();
  } else {
    key = '\0';
  }
  Wire.print(key);
  Serial.print("REQUEST: ");
  Serial.println(key);
}

void onReceive(int bytesRead) {
  char cmd;
  while (Wire.available()) {
    cmd = Wire.read();
    Serial.print("RECEIVE: ");
    Serial.println(cmd);
    switch (cmd) {
      case 0x0:
        // TODO: Implement
        // i2cBuffer.flush();
        break;
      case 0x10:
        setBacklight(false);
        break;
      case 0x11:
        setBacklight(true);
        break;
      case 0x12:
        setBacklight(!backlightState);
        break;
      default:
        Serial.print("Unknown command: ");
        Serial.println(cmd);
    }
  }
}

void readMatrix() {
  int delayTime = 0;
  // iterate the columns
  for (int colIndex = 0; colIndex < colCount; colIndex++) {
    // col: set to output to low
    uint8_t curCol = cols[colIndex];
    pinMode(curCol, OUTPUT);
    digitalWrite(curCol, LOW);

    // row: iterate through the rows
    for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {
      uint8_t rowCol = rows[rowIndex];
      pinMode(rowCol, INPUT_PULLUP);
      delay(1);  // arduino is not fast enough to switch input/output modes

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
    // disable the column
    pinMode(curCol, INPUT);
  }

  // TODO: Move this
  if (keyPressed(0, 2)) {
    symbolSelected = !symbolSelected;
  }
}

void printMatrix() {
  for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {
    for (int colIndex = 0; colIndex < colCount; colIndex++) {
      // we only want to print if the key is pressed and it is a printable
      // character
      if (keyPressed(colIndex, rowIndex) &&
          isPrintableKey(colIndex, rowIndex)) {
        char toPrint;
        // Symbol key modifier
        if (symbolSelected) {
          toPrint = char(keyboardSymbol[colIndex][rowIndex]);
          symbolSelected = false;
          // Shift Key Modifier
        } else if (keyActive(1, 6) || keyActive(2, 3)) {
          toPrint = (char)((int)char(keyboard[colIndex][rowIndex]) - 32);
        } else {
          toPrint = char(keyboard[colIndex][rowIndex]);
        }

        // Add key to I2C buffer
        Serial.print("KEY: ");
        Serial.println(toPrint);
        i2cBuffer.push(toPrint);
      }
    }
  }
}

bool keyPressed(int colIndex, int rowIndex) {
  return changedValue[colIndex][rowIndex] && keys[colIndex][rowIndex];
}

bool keyActive(int colIndex, int rowIndex) { return keys[colIndex][rowIndex]; }

bool isPrintableKey(int colIndex, int rowIndex) {
  return keyboardSymbol[colIndex][rowIndex] != '\0' ||
         keyboard[colIndex][rowIndex] != '\0';
}

void setBacklight(bool state) {
  digitalWrite(LITE, state);
  backlightState = state;
}
