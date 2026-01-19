#pragma once

#include "App.h"

/**
 * Magic 8-Ball Fortune Teller
 * Ask a yes/no question and get a mystical answer!
 * Classic novelty fortune-telling toy.
 */
class Magic8BallApp final : public App {
  bool shouldExitFlag = false;
  const char* currentAnswer = nullptr;
  bool showingAnswer = false;
  unsigned long answerShownTime = 0;

  const char* getRandomAnswer();

 public:
  explicit Magic8BallApp(GfxRenderer& renderer, MappedInputManager& mappedInput) : App(renderer, mappedInput) {}

  const char* getName() const override;
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render() override;
  bool shouldExit() const override;
};
