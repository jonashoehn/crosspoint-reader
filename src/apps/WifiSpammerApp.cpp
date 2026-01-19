#include "WifiSpammerApp.h"

#include <Arduino.h>
#include <GfxRenderer.h>

#include "MappedInputManager.h"
#include "fontIds.h"

extern "C" {
#include "esp_wifi.h"
esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second);
esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void* buffer, int len, bool en_sys_seq);
}

// WiFi channels to use
const uint8_t channels[] = {1, 6, 11};  // Common non-overlapping channels
const bool wpa2 = false;                // Open networks for simplicity

// SSID list (reduced for flash space)
const char ssids[] PROGMEM = {
    "FBI Surveillance Van #3\n"
    "Pretty Fly for a WiFi\n"
    "Get Off My LAN\n"
    "404 Network Unavailable\n"
    "Loading...\n"
    "Searching...\n"
    "Abraham Linksys\n"
    "Martin Router King\n"
    "Bill Wi the Science Fi\n"
    "Silence of the LANs\n"
    "It Burns When IP\n"
    "LAN Solo\n"
    "The Promised LAN\n"
    "House LANister\n"
    "Winternet is Coming\n"
    "Dropbox\n"
    "No More Mr WiFi\n"
    "I Believe I Can WiFi\n"
    "Tell My WiFi Love Her\n"
    "🌮 Free Tacos Here 🌮\n"
    "Click Here for Virus\n"
    "Free Public WiFi\n"
    "Starbucks Free WiFi\n"
    "Airport Free WiFi\n"
};

// Beacon frame template
uint8_t beaconPacket[109] = {
    /*  0 - 3  */ 0x80, 0x00, 0x00, 0x00,              // Type/Subtype: management beacon frame
    /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // Destination: broadcast
    /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,  // Source
    /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,  // Source

    // Fixed parameters
    /* 22 - 23 */ 0x00, 0x00,                                      // Fragment & sequence number
    /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00,  // Timestamp
    /* 32 - 33 */ 0xe8, 0x03,                                      // Interval: every 1s
    /* 34 - 35 */ 0x21, 0x00,                                      // Capabilities (open network)

    // SSID parameters
    /* 36 - 37 */ 0x00, 0x20,  // Tag: Set SSID length, Tag length: 32
    /* 38 - 69 */ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,  // SSID

    // Supported Rates
    /* 70 - 71 */ 0x01, 0x08,  // Tag: Supported Rates, Tag length: 8
    /* 72 */ 0x82,             // 1(B)
    /* 73 */ 0x84,             // 2(B)
    /* 74 */ 0x8b,             // 5.5(B)
    /* 75 */ 0x96,             // 11(B)
    /* 76 */ 0x24,             // 18
    /* 77 */ 0x30,             // 24
    /* 78 */ 0x48,             // 36
    /* 79 */ 0x6c,             // 54

    // Current Channel
    /* 80 - 81 */ 0x03, 0x01,  // Channel set, length
    /* 82 */ 0x01,             // Current Channel
};

const char* WifiSpammerApp::getName() const {
  return "WiFi Spammer";
}

void WifiSpammerApp::onEnter() {
  shouldExitFlag = false;
  isRunning = false;
  packetCounter = 0;
  packetsPerSecond = 0;
  lastPacketRateUpdate = millis();
  lastAttackTime = millis();
  channelIndex = 0;
  currentChannel = channels[0];
}

void WifiSpammerApp::onExit() {
  if (isRunning) {
    stopWifi();
  }
}

void WifiSpammerApp::loop() {
  const unsigned long currentTime = millis();

  // Handle input
  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    shouldExitFlag = true;
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Confirm)) {
    if (isRunning) {
      stopWifi();
    } else {
      initWifi();
    }
  }

  // Send beacons if running
  if (isRunning && currentTime - lastAttackTime > 100) {
    lastAttackTime = currentTime;
    sendBeacons();
  }

  // Update packet rate stats
  if (currentTime - lastPacketRateUpdate > 1000) {
    lastPacketRateUpdate = currentTime;
    packetsPerSecond = packetCounter;
    packetCounter = 0;
  }
}

void WifiSpammerApp::render() {
  renderer.clearScreen();

  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  // Draw title
  renderer.drawCenteredText(UI_12_FONT_ID, 30, "WiFi SSID Spammer", true, EpdFontFamily::BOLD);

  // Draw status
  const int centerY = pageHeight / 2 - renderer.getLineHeight(UI_10_FONT_ID) * 3;
  const char* status = isRunning ? "RUNNING" : "STOPPED";
  renderer.drawCenteredText(UI_10_FONT_ID, centerY, status, true, EpdFontFamily::BOLD);

  // Draw stats
  if (isRunning) {
    char statsLine1[64];
    snprintf(statsLine1, sizeof(statsLine1), "Channel: %d", currentChannel);
    renderer.drawCenteredText(UI_10_FONT_ID, centerY + renderer.getLineHeight(UI_10_FONT_ID) + 10, statsLine1);

    char statsLine2[64];
    snprintf(statsLine2, sizeof(statsLine2), "Packets/sec: %lu", packetsPerSecond);
    renderer.drawCenteredText(UI_10_FONT_ID, centerY + renderer.getLineHeight(UI_10_FONT_ID) * 2 + 15, statsLine2);

    char statsLine3[64];
    snprintf(statsLine3, sizeof(statsLine3), "SSIDs: %d", 24);  // Number of SSIDs in list
    renderer.drawCenteredText(UI_10_FONT_ID, centerY + renderer.getLineHeight(UI_10_FONT_ID) * 3 + 20, statsLine3);
  } else {
    renderer.drawCenteredText(SMALL_FONT_ID, centerY + renderer.getLineHeight(UI_10_FONT_ID) + 10,
                              "Press Confirm to Start");
  }

  // Draw warning
  renderer.drawCenteredText(SMALL_FONT_ID, pageHeight - 90, "Educational/Privacy Tool Only");
  renderer.drawCenteredText(SMALL_FONT_ID, pageHeight - 75, "Creates fake WiFi SSIDs");

  // Draw button hints
  const auto labels = mappedInput.mapLabels("« Exit", isRunning ? "Stop" : "Start", "", "");
  renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}

bool WifiSpammerApp::shouldExit() const {
  return shouldExitFlag;
}

void WifiSpammerApp::initWifi() {
  randomMac();
  WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_channel(channels[0], WIFI_SECOND_CHAN_NONE);
  currentChannel = channels[0];
  channelIndex = 0;
  isRunning = true;
}

void WifiSpammerApp::stopWifi() {
  isRunning = false;
  WiFi.mode(WIFI_MODE_NULL);
}

void WifiSpammerApp::nextChannel() {
  if (sizeof(channels) < 2) {
    return;
  }

  channelIndex++;
  if (channelIndex >= sizeof(channels)) {
    channelIndex = 0;
  }

  const uint8_t ch = channels[channelIndex];
  if (ch >= 1 && ch <= 14) {
    currentChannel = ch;
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
  }
}

void WifiSpammerApp::randomMac() {
  for (int i = 0; i < 6; i++) {
    macAddr[i] = random(256);
  }
}

void WifiSpammerApp::sendBeacons() {
  // Go to next channel
  nextChannel();

  // Empty SSID buffer for reset
  char emptySSID[32];
  for (int i = 0; i < 32; i++) {
    emptySSID[i] = ' ';
  }

  // Parse and send each SSID
  const int ssidsLen = strlen_P(ssids);
  int i = 0;
  int ssidNum = 1;

  while (i < ssidsLen) {
    // Read SSID until newline
    int j = 0;
    char tmp;
    do {
      tmp = pgm_read_byte(ssids + i + j);
      j++;
    } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

    const uint8_t ssidLen = j - 1;

    // Set MAC address (increment last byte for uniqueness)
    macAddr[5] = ssidNum;
    ssidNum++;

    // Write MAC address into beacon frame
    memcpy(&beaconPacket[10], macAddr, 6);
    memcpy(&beaconPacket[16], macAddr, 6);

    // Reset SSID field
    memcpy(&beaconPacket[38], emptySSID, 32);

    // Write new SSID into beacon frame
    memcpy_P(&beaconPacket[38], &ssids[i], ssidLen);

    // Set channel for beacon frame
    beaconPacket[82] = currentChannel;

    // Send packet (send 3 times for reliability, reduced from original for speed)
    constexpr int packetSize = 83;  // Size without WPA2 fields
    for (int k = 0; k < 2; k++) {
      if (esp_wifi_80211_tx(WIFI_IF_STA, beaconPacket, packetSize, false) == ESP_OK) {
        packetCounter++;
      }
      delay(1);
    }

    i += j;
  }
}
