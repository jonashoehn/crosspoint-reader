#include "AppRunnerActivity.h"

void AppRunnerActivity::taskTrampoline(void* param) {
  auto* self = static_cast<AppRunnerActivity*>(param);
  self->displayTaskLoop();
}

void AppRunnerActivity::onEnter() {
  Activity::onEnter();

  renderingMutex = xSemaphoreCreateMutex();

  if (app) {
    app->onEnter();
  }

  // Trigger first update
  updateRequired = true;

  xTaskCreate(&AppRunnerActivity::taskTrampoline, "AppRunnerTask",
              4096,               // Stack size
              this,               // Parameters
              1,                  // Priority
              &displayTaskHandle  // Task handle
  );
}

void AppRunnerActivity::onExit() {
  Activity::onExit();

  // Wait until not rendering to delete task
  xSemaphoreTake(renderingMutex, portMAX_DELAY);
  if (displayTaskHandle) {
    vTaskDelete(displayTaskHandle);
    displayTaskHandle = nullptr;
  }
  vSemaphoreDelete(renderingMutex);
  renderingMutex = nullptr;

  if (app) {
    app->onExit();
  }
}

void AppRunnerActivity::loop() {
  if (app) {
    app->loop();

    // Check if app wants to exit
    if (app->shouldExit()) {
      onExitCallback();
    }
  }
}

void AppRunnerActivity::displayTaskLoop() {
  while (true) {
    if (app) {
      xSemaphoreTake(renderingMutex, portMAX_DELAY);
      app->render();
      xSemaphoreGive(renderingMutex);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Render at ~10fps
  }
}
