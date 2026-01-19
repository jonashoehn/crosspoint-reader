#pragma once

#include "App.h"

/**
 * Simple "Hello World" app demonstrating the app framework.
 * Displays a greeting message and the current uptime.
 */
class HelloWorldApp final : public App {
  bool shouldExitFlag = false;

 public:
  explicit HelloWorldApp(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : App(renderer, mappedInput) {}

  const char* getName() const override;
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render() override;
  bool shouldExit() const override;
};
