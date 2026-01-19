#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <functional>
#include <memory>

#include "apps/App.h"

#include "../Activity.h"

/**
 * Activity wrapper that runs an App instance.
 * Manages the app lifecycle and integrates it into the activity system.
 */
class AppRunnerActivity final : public Activity {
  std::unique_ptr<App> app;
  TaskHandle_t displayTaskHandle = nullptr;
  SemaphoreHandle_t renderingMutex = nullptr;
  bool updateRequired = false;
  const std::function<void()> onExitCallback;

  static void taskTrampoline(void* param);
  [[noreturn]] void displayTaskLoop();

 public:
  explicit AppRunnerActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, App* app,
                             const std::function<void()>& onExitCallback)
      : Activity("AppRunner", renderer, mappedInput), app(app), onExitCallback(onExitCallback) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
};
