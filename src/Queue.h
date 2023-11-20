//  SPDX-FileCopyrightText: 2023 Robert Grizzell
//  SPDX-License-Identifier: MIT
#include "Arduino.h"

#define MAX_SIZE 16

class Queue {
 private:
  int head;
  int tail;
  int count;
  uint8_t buffer[MAX_SIZE];

 public:
  Queue();
  ~Queue();
  void push(uint8_t data);
  uint8_t pop();
  int size();
};
