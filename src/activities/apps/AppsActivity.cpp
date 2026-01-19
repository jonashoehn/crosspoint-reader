#include "AppsActivity.h"

#include <GfxRenderer.h>

#include "AppRunnerActivity.h"
#include "MappedInputManager.h"
#include "apps/AppRegistry.h"
#include "fontIds.h"

void AppsActivity::taskTrampoline(void* param) {
  auto* self = static_cast<AppsActivity*>(param);
  self->displayTaskLoop();
}

void AppsActivity::onEnter() {
  Activity::onEnter();
  renderingMutex = xSemaphoreCreateMutex();

  // Reset selection to first item
  selectedAppIndex = 0;

  // Trigger first update
  updateRequired = true;

  xTaskCreate(&AppsActivity::taskTrampoline, "AppsActivityTask",
              4096,               // Stack size
              this,               // Parameters
              1,                  // Priority
              &displayTaskHandle  // Task handle
  );
}

void AppsActivity::onExit() {
  ActivityWithSubactivity::onExit();

  // Wait until not rendering to delete task
  xSemaphoreTake(renderingMutex, portMAX_DELAY);
  if (displayTaskHandle) {
    vTaskDelete(displayTaskHandle);
    displayTaskHandle = nullptr;
  }
  vSemaphoreDelete(renderingMutex);
  renderingMutex = nullptr;
}

void AppsActivity::loop() {
  if (subActivity) {
    subActivity->loop();
    return;
  }

  // Handle actions with early return
  if (mappedInput.wasPressed(MappedInputManager::Button::Confirm)) {
    launchSelectedApp();
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    onGoHome();
    return;
  }

  // Handle navigation
  const auto& registry = AppRegistry::getInstance();
  const int appCount = registry.getAppCount();

  if (appCount == 0) {
    // No apps available, nothing to navigate
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Up) ||
      mappedInput.wasPressed(MappedInputManager::Button::Left)) {
    // Move selection up (with wrap-around)
    selectedAppIndex = (selectedAppIndex > 0) ? (selectedAppIndex - 1) : (appCount - 1);
    updateRequired = true;
  } else if (mappedInput.wasPressed(MappedInputManager::Button::Down) ||
             mappedInput.wasPressed(MappedInputManager::Button::Right)) {
    // Move selection down (with wrap around)
    selectedAppIndex = (selectedAppIndex < appCount - 1) ? (selectedAppIndex + 1) : 0;
    updateRequired = true;
  }
}

void AppsActivity::launchSelectedApp() {
  const auto& registry = AppRegistry::getInstance();
  const int appCount = registry.getAppCount();

  if (selectedAppIndex < 0 || selectedAppIndex >= appCount) {
    return;
  }

  // Create app instance
  App* app = registry.createApp(selectedAppIndex, renderer, mappedInput);
  if (!app) {
    return;
  }

  // Launch app in AppRunnerActivity
  xSemaphoreTake(renderingMutex, portMAX_DELAY);
  exitActivity();
  enterNewActivity(new AppRunnerActivity(renderer, mappedInput, app, [this] {
    exitActivity();
    updateRequired = true;
  }));
  xSemaphoreGive(renderingMutex);
}

void AppsActivity::displayTaskLoop() {
  while (true) {
    if (updateRequired && !subActivity) {
      updateRequired = false;
      xSemaphoreTake(renderingMutex, portMAX_DELAY);
      render();
      xSemaphoreGive(renderingMutex);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void AppsActivity::render() const {
  renderer.clearScreen();

  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  // Draw header
  renderer.drawCenteredText(UI_12_FONT_ID, 15, "Apps", true, EpdFontFamily::BOLD);

  const auto& registry = AppRegistry::getInstance();
  const int appCount = registry.getAppCount();

  if (appCount == 0) {
    // No apps available
    const int y = pageHeight / 2 - renderer.getLineHeight(UI_10_FONT_ID);
    renderer.drawCenteredText(UI_10_FONT_ID, y, "No apps available");
    renderer.drawCenteredText(UI_10_FONT_ID, y + renderer.getLineHeight(UI_10_FONT_ID) + 5,
                              "Add apps to AppRegistry in main.cpp");
  } else {
    // Draw selection highlight
    renderer.fillRect(0, 60 + selectedAppIndex * 30 - 2, pageWidth - 1, 30);

    // Draw all apps
    const auto& apps = registry.getApps();
    for (int i = 0; i < appCount; i++) {
      const int appY = 60 + i * 30;  // 30 pixels between apps
      const bool isSelected = (i == selectedAppIndex);

      // Draw app name
      renderer.drawText(UI_10_FONT_ID, 20, appY, apps[i].name, !isSelected);
    }
  }

  // Draw help text
  const auto labels = mappedInput.mapLabels("« Back", "Launch", "", "");
  renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}
