#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <functional>

#include "../ActivityWithSubactivity.h"

/**
 * Apps menu activity that displays available apps and launches them.
 * Similar to SettingsActivity but for launching apps instead of configuring settings.
 */
class AppsActivity final : public ActivityWithSubactivity {
  TaskHandle_t displayTaskHandle = nullptr;
  SemaphoreHandle_t renderingMutex = nullptr;
  bool updateRequired = false;
  int selectedAppIndex = 0;
  const std::function<void()> onGoHome;

  static void taskTrampoline(void* param);
  [[noreturn]] void displayTaskLoop();
  void render() const;
  void launchSelectedApp();

 public:
  explicit AppsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, const std::function<void()>& onGoHome)
      : ActivityWithSubactivity("Apps", renderer, mappedInput), onGoHome(onGoHome) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
};
