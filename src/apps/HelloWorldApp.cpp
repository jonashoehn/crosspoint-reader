#include "HelloWorldApp.h"

#include <Arduino.h>
#include <GfxRenderer.h>

#include "MappedInputManager.h"
#include "fontIds.h"

const char* HelloWorldApp::getName() const {
  return "Hello World";
}

void HelloWorldApp::onEnter() {
  shouldExitFlag = false;
}

void HelloWorldApp::onExit() {
  // Clean up if needed
}

void HelloWorldApp::loop() {
  // Check for back button to exit
  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    shouldExitFlag = true;
  }
}

void HelloWorldApp::render() {
  renderer.clearScreen();

  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  // Draw title
  renderer.drawCenteredText(UI_12_FONT_ID, 30, "Hello World App", true, EpdFontFamily::BOLD);

  // Draw greeting message
  const int centerY = pageHeight / 2 - renderer.getLineHeight(UI_10_FONT_ID) * 2;
  renderer.drawCenteredText(UI_10_FONT_ID, centerY, "Welcome to the CrossPoint Apps System!");

  // Draw uptime
  const unsigned long uptimeSeconds = millis() / 1000;
  const unsigned long hours = uptimeSeconds / 3600;
  const unsigned long minutes = (uptimeSeconds % 3600) / 60;
  const unsigned long seconds = uptimeSeconds % 60;

  char uptimeStr[64];
  snprintf(uptimeStr, sizeof(uptimeStr), "Uptime: %02luh %02lum %02lus", hours, minutes, seconds);
  renderer.drawCenteredText(UI_10_FONT_ID, centerY + renderer.getLineHeight(UI_10_FONT_ID) + 10, uptimeStr);

  // Draw instructions
  renderer.drawCenteredText(SMALL_FONT_ID, pageHeight - 80, "This is a simple proof-of-concept app.");
  renderer.drawCenteredText(SMALL_FONT_ID, pageHeight - 65, "You can add more fun apps here!");

  // Draw button hints
  const auto labels = mappedInput.mapLabels("« Exit", "", "", "");
  renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}

bool HelloWorldApp::shouldExit() const {
  return shouldExitFlag;
}
