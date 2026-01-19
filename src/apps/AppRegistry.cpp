#include "AppRegistry.h"

#include "App.h"

AppRegistry& AppRegistry::getInstance() {
  static AppRegistry instance;
  return instance;
}

void AppRegistry::registerApp(const char* name, AppFactory factory) {
  apps.push_back({name, factory});
}

const std::vector<AppRegistry::AppInfo>& AppRegistry::getApps() const {
  return apps;
}

int AppRegistry::getAppCount() const {
  return static_cast<int>(apps.size());
}

App* AppRegistry::createApp(int index, GfxRenderer& renderer, MappedInputManager& mappedInput) const {
  if (index < 0 || index >= static_cast<int>(apps.size())) {
    return nullptr;
  }
  return apps[index].factory(renderer, mappedInput);
}
