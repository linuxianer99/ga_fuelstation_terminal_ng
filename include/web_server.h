#pragma once

#include <FS.h>
#include <WebServer.h>

#include "config.h"

namespace fuelstation {

class FuelStationController;

class WebServerController {
 public:
  void begin(const RuntimeConfig& config, FuelStationController& station);
  void update();
  void appendLog(const String& line);

 private:
  bool authenticated();
  void handleStatus();
  void handleConfigDownload();
  void handleConfigUpload();
  void handleFirmwareUpload();
  String statusJson() const;
  String uptime() const;

  WebServer server_{80};
  RuntimeConfig config_;
  FuelStationController* station_ = nullptr;
  String log_;
  File configUpload_;
};

}  // namespace fuelstation