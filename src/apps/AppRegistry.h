#pragma once

#include <functional>
#include <vector>

class App;
class GfxRenderer;
class MappedInputManager;

/**
 * Factory function type for creating app instances.
 * Takes renderer and input manager, returns a new App instance.
 */
using AppFactory = std::function<App*(GfxRenderer&, MappedInputManager&)>;

/**
 * Registry for managing available apps.
 * Apps are registered with their factory functions and can be queried/created.
 */
class AppRegistry {
 public:
  struct AppInfo {
    const char* name;
    AppFactory factory;
  };

 private:
  std::vector<AppInfo> apps;

  // Singleton pattern
  AppRegistry() = default;

 public:
  // Prevent copying
  AppRegistry(const AppRegistry&) = delete;
  AppRegistry& operator=(const AppRegistry&) = delete;

  /**
   * Get the singleton instance of the app registry.
   */
  static AppRegistry& getInstance();

  /**
   * Register a new app with the registry.
   * @param name Display name of the app
   * @param factory Factory function to create app instances
   */
  void registerApp(const char* name, AppFactory factory);

  /**
   * Get the list of all registered apps.
   * @return Vector of app info structures
   */
  const std::vector<AppInfo>& getApps() const;

  /**
   * Get the number of registered apps.
   * @return Number of apps
   */
  int getAppCount() const;

  /**
   * Create an instance of an app by index.
   * @param index Index of the app in the registry
   * @param renderer Renderer instance to pass to the app
   * @param mappedInput Input manager instance to pass to the app
   * @return New app instance, or nullptr if index is invalid
   */
  App* createApp(int index, GfxRenderer& renderer, MappedInputManager& mappedInput) const;
};
