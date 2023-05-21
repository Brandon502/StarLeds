#include <Arduino.h>

#pragma once

class Module {

public:
  const char *name;
  bool success;
  bool enabled;

  Module(const char *name) {
    this->name = name;
    enabled = true;
  }

  virtual void setup() {
    success = true;
  }

  virtual void loop() {
  }

  virtual void testManager() {}
  virtual void performanceManager() {}
  virtual void dataSizeManager() {}
  virtual void codeSizeManager() {}
};