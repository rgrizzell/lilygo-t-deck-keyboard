//  SPDX-FileCopyrightText: 2023 Robert Grizzell
//  SPDX-License-Identifier: MIT
#include "Queue.h"

Queue::Queue() {
  head = 0;
  tail = 0;
  count = 0;
}

Queue::~Queue() {}

void Queue::push(uint8_t data) {
  if (count == MAX_SIZE) {
    return;
  } else {
    count++;
    if (count > 1) {
      tail++;
      tail %= MAX_SIZE;
    }

    buffer[tail] = data;
  }
}

uint8_t Queue::pop() {
  if (count == 0) {
    return '\x00';
  } else {
    count--;
    uint8_t data = buffer[head];
    if (count >= 1) {
      head++;
      head %= MAX_SIZE;
    }

    return data;
  }
}

int Queue::size() { return count; }
