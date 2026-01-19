#pragma once

#include <string>

class GfxRenderer;
class MappedInputManager;

/**
 * Base interface for all apps in the system.
 * Apps are lightweight, modular programs that can be launched from the Apps menu.
 */
class App {
 protected:
  GfxRenderer& renderer;
  MappedInputManager& mappedInput;

 public:
  explicit App(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : renderer(renderer), mappedInput(mappedInput) {}
  virtual ~App() = default;

  /**
   * Get the display name of this app.
   * @return The name to show in the Apps menu
   */
  virtual const char* getName() const = 0;

  /**
   * Called when the app is entered/launched.
   * Use this to initialize state and resources.
   */
  virtual void onEnter() = 0;

  /**
   * Called when the app is exited.
   * Use this to clean up resources.
   */
  virtual void onExit() = 0;

  /**
   * Main loop function, called repeatedly while app is active.
   * Handle input and update state here.
   */
  virtual void loop() = 0;

  /**
   * Render the app UI to the screen.
   * This is typically called from the loop when an update is needed.
   */
  virtual void render() = 0;

  /**
   * Check if the user has requested to exit the app.
   * @return true if the app should exit, false otherwise
   */
  virtual bool shouldExit() const = 0;
};
