#include "Magic8BallApp.h"

#include <Arduino.h>
#include <GfxRenderer.h>

#include "MappedInputManager.h"
#include "fontIds.h"

// Classic Magic 8-Ball answers (20 traditional responses)
// Positive answers (10)
const char* const POSITIVE_ANSWERS[] PROGMEM = {
    "It is certain",
    "It is decidedly so",
    "Without a doubt",
    "Yes definitely",
    "You may rely on it",
    "As I see it, yes",
    "Most likely",
    "Outlook good",
    "Yes",
    "Signs point to yes",
};

// Non-committal answers (5)
const char* const NEUTRAL_ANSWERS[] PROGMEM = {
    "Reply hazy, try again",
    "Ask again later",
    "Better not tell you now",
    "Cannot predict now",
    "Concentrate and ask again",
};

// Negative answers (5)
const char* const NEGATIVE_ANSWERS[] PROGMEM = {
    "Don't count on it",
    "My reply is no",
    "My sources say no",
    "Outlook not so good",
    "Very doubtful",
};

const char* Magic8BallApp::getName() const {
  return "Magic 8-Ball";
}

void Magic8BallApp::onEnter() {
  shouldExitFlag = false;
  currentAnswer = nullptr;
  showingAnswer = false;
  answerShownTime = 0;
}

void Magic8BallApp::onExit() {
  // Clean up if needed
}

void Magic8BallApp::loop() {
  // Handle input
  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    shouldExitFlag = true;
    return;
  }

  // Shake the ball (ask for answer)
  if (mappedInput.wasPressed(MappedInputManager::Button::Confirm)) {
    currentAnswer = getRandomAnswer();
    showingAnswer = true;
    answerShownTime = millis();
  }

  // Reset answer after showing for a while (optional, or keep it displayed)
  // Commented out to keep answer visible until next shake
  // if (showingAnswer && millis() - answerShownTime > 5000) {
  //   showingAnswer = false;
  //   currentAnswer = nullptr;
  // }
}

void Magic8BallApp::render() {
  renderer.clearScreen();

  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  // Draw title
  renderer.drawCenteredText(UI_12_FONT_ID, 20, "Magic 8-Ball", true, EpdFontFamily::BOLD);

  // Draw the 8-ball circle (simplified - just a rectangle frame for e-ink)
  const int ballSize = 180;
  const int ballX = (pageWidth - ballSize) / 2;
  const int ballY = 80;

  // Draw ball outline
  renderer.fillRect(ballX, ballY, ballSize, ballSize);
  renderer.drawRect(ballX + 2, ballY + 2, ballSize - 4, ballSize - 4, false);

  // Draw inner window (triangle would be complex, use circle/rect)
  const int windowSize = 120;
  const int windowX = (pageWidth - windowSize) / 2;
  const int windowY = ballY + (ballSize - windowSize) / 2;
  renderer.fillRect(windowX, windowY, windowSize, windowSize, false);

  if (showingAnswer && currentAnswer) {
    // Draw answer text in the window
    // Word wrap the answer
    String answerStr = currentAnswer;
    int lineCount = 0;
    String lines[3];  // Max 3 lines
    int startIdx = 0;

    // Simple word wrapping
    while (startIdx < answerStr.length() && lineCount < 3) {
      int endIdx = answerStr.length();
      String testLine = answerStr.substring(startIdx);

      // Find last space that fits
      while (renderer.getTextWidth(UI_10_FONT_ID, testLine.c_str()) > windowSize - 20 && testLine.length() > 0) {
        int lastSpace = testLine.lastIndexOf(' ');
        if (lastSpace > 0) {
          testLine = testLine.substring(0, lastSpace);
          endIdx = startIdx + lastSpace;
        } else {
          break;
        }
      }

      if (endIdx == answerStr.length()) {
        lines[lineCount++] = testLine;
        break;
      } else {
        lines[lineCount++] = answerStr.substring(startIdx, endIdx);
        startIdx = endIdx + 1;
      }
    }

    // Draw centered lines
    const int lineHeight = renderer.getLineHeight(UI_10_FONT_ID);
    const int totalHeight = lineHeight * lineCount;
    int textY = windowY + (windowSize - totalHeight) / 2;

    for (int i = 0; i < lineCount; i++) {
      const int textWidth = renderer.getTextWidth(UI_10_FONT_ID, lines[i].c_str());
      const int textX = windowX + (windowSize - textWidth) / 2;
      renderer.drawText(UI_10_FONT_ID, textX, textY, lines[i].c_str());
      textY += lineHeight;
    }
  } else {
    // Draw "8" in the window when not showing answer
    renderer.drawCenteredText(UI_12_FONT_ID, windowY + (windowSize - renderer.getLineHeight(UI_12_FONT_ID)) / 2, "8",
                              false, EpdFontFamily::BOLD);
  }

  // Draw instructions
  if (!showingAnswer) {
    renderer.drawCenteredText(SMALL_FONT_ID, pageHeight - 95, "Think of a yes/no question...");
    renderer.drawCenteredText(SMALL_FONT_ID, pageHeight - 80, "Press Confirm to shake!");
  } else {
    renderer.drawCenteredText(SMALL_FONT_ID, pageHeight - 80, "Shake again for another answer");
  }

  // Draw button hints
  const auto labels = mappedInput.mapLabels("« Exit", "Shake", "", "");
  renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}

bool Magic8BallApp::shouldExit() const {
  return shouldExitFlag;
}

const char* Magic8BallApp::getRandomAnswer() {
  // Weighted random selection to match classic Magic 8-Ball distribution
  const int randomVal = random(20);  // 0-19

  if (randomVal < 10) {
    // 50% positive
    return POSITIVE_ANSWERS[random(10)];
  } else if (randomVal < 15) {
    // 25% neutral
    return NEUTRAL_ANSWERS[random(5)];
  } else {
    // 25% negative
    return NEGATIVE_ANSWERS[random(5)];
  }
}
