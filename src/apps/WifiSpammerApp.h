#pragma once

#include <WiFi.h>

#include "App.h"

/**
 * WiFi SSID Beacon Spammer
 * Sends fake WiFi beacon frames for educational/privacy purposes.
 * Creates phantom SSIDs visible in WiFi scans.
 */
class WifiSpammerApp final : public App {
  bool shouldExitFlag = false;
  bool isRunning = false;
  uint32_t packetCounter = 0;
  uint32_t lastPacketRateUpdate = 0;
  uint32_t lastAttackTime = 0;
  uint32_t packetsPerSecond = 0;
  uint8_t channelIndex = 0;
  uint8_t currentChannel = 1;
  uint8_t macAddr[6];

  void initWifi();
  void stopWifi();
  void nextChannel();
  void randomMac();
  void sendBeacons();

 public:
  explicit WifiSpammerApp(GfxRenderer& renderer, MappedInputManager& mappedInput) : App(renderer, mappedInput) {}

  const char* getName() const override;
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render() override;
  bool shouldExit() const override;
};
