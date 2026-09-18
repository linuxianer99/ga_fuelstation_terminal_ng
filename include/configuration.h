#pragma once

#include "config.h"

namespace fuelstation {

class Configuration {
 public:
  bool begin();
  const RuntimeConfig& values() const;
  bool isFilesystemReady() const;

 private:
  bool loadJson();

  RuntimeConfig values_;
  bool filesystemReady_ = false;
};

}  // namespace fuelstation
