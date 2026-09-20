#include "web_server.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Update.h>

#include "fuel_station_controller.h"

namespace fuelstation {

namespace {
const char STATUS_PAGE[] = R"HTML(<!doctype html><html><head><meta charset="utf-8"><title>Fuel Station</title><style>body{font-family:sans-serif;max-width:800px;margin:2rem auto;padding:0 1rem}pre{background:#eee;padding:1rem;white-space:pre-wrap;max-height:300px;overflow-y:auto;border:1px solid #ccc;border-radius:4px}form{margin:1rem 0}</style></head><body><h1 id="name">Fuel Station</h1><p>Status: <b id="state">...</b></p><p>Uptime: <span id="uptime">...</span></p><p><a href="/config.json">config.json herunterladen</a></p><form method="post" action="/config" enctype="multipart/form-data"><input type="file" name="config" accept=".json" required><button>Konfiguration hochladen</button></form><form method="post" action="/update" enctype="multipart/form-data"><input type="file" name="firmware" accept=".bin" required><button>Firmware hochladen</button></form><form method="post" action="/restart"><button>Restart</button></form><h2>SerialLog</h2><pre id="log">...</pre><script>async function refresh(){let s=await(await fetch('/api/status')).json();name.textContent=s.station;state.textContent=s.status;uptime.textContent=s.uptime;log.textContent=s.log}refresh();setInterval(refresh,5000)</script></body></html>)HTML";
}

void WebServerController::begin(const RuntimeConfig& config, FuelStationController& station) {
  config_ = config;
  station_ = &station;
  server_.on("/", HTTP_GET, [this]() {
    if (authenticated()) server_.send(200, "text/html", STATUS_PAGE);
  });
  server_.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
  server_.on("/config.json", HTTP_GET, [this]() { handleConfigDownload(); });
  server_.on("/config", HTTP_POST, [this]() {
    if (authenticated()) server_.send(200, "text/plain", "Konfiguration gespeichert. Bitte neu starten.");
  }, [this]() { handleConfigUpload(); });
  server_.on("/restart", HTTP_POST, [this]() {
    if (!authenticated()) return;
    server_.send(200, "text/plain", "Restart wird ausgefuehrt");
    delay(100);
    ESP.restart();
  });
  server_.on("/update", HTTP_POST, [this]() {
    if (!authenticated()) return;
    if (!Update.end(true)) {
      server_.send(500, "text/plain", "Firmware-Update fehlgeschlagen");
      return;
    }
    server_.send(200, "text/plain", "Firmware installiert. Neustart wird ausgefuehrt");
    delay(100);
    ESP.restart();
  }, [this]() { handleFirmwareUpload(); });
  server_.begin();
  appendLog("Webserver auf Port 80 gestartet");
}

void WebServerController::update() {
  server_.handleClient();
}

void WebServerController::appendLog(const String& line) {
  log_ += line + "\n";
  if (log_.length() > 4096) log_ = log_.substring(log_.length() - 4096);
}

bool WebServerController::authenticated() {
  if (server_.authenticate(config_.httpUser.c_str(), config_.httpPassword.c_str())) return true;
  server_.requestAuthentication();
  return false;
}

void WebServerController::handleStatus() {
  if (authenticated()) server_.send(200, "application/json", statusJson());
}

void WebServerController::handleConfigDownload() {
  if (!authenticated()) return;
  if (!LittleFS.exists("/config.json")) {
    server_.send(404, "text/plain", "config.json nicht gefunden");
    return;
  }
  File file = LittleFS.open("/config.json", FILE_READ);
  server_.sendHeader("Content-Disposition", "attachment; filename=config.json");
  server_.streamFile(file, "application/json");
  file.close();
}

void WebServerController::handleConfigUpload() {
  if (!authenticated()) return;
  HTTPUpload& upload = server_.upload();
  if (upload.status == UPLOAD_FILE_START) {
    if (upload.filename != "config.json") return;
    configUpload_ = LittleFS.open("/config.json.tmp", FILE_WRITE);
  } else if (upload.status == UPLOAD_FILE_WRITE && configUpload_) {
    configUpload_.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END && configUpload_) {
    configUpload_.close();
    File validation = LittleFS.open("/config.json.tmp", FILE_READ);
    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, validation);
    validation.close();
    if (error) {
      LittleFS.remove("/config.json.tmp");
      return;
    }
    LittleFS.remove("/config.json");
    LittleFS.rename("/config.json.tmp", "/config.json");
  }
}

void WebServerController::handleFirmwareUpload() {
  if (!authenticated()) return;
  HTTPUpload& upload = server_.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Update.begin(UPDATE_SIZE_UNKNOWN);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Update.write(upload.buf, upload.currentSize);
  }
}

String WebServerController::statusJson() const {
  JsonDocument doc;
  doc["station"] = config_.stationName;
  doc["terminal_id"] = config_.terminalId;
  doc["status"] = station_->state() == StationState::PumpRunning ? "pump_running" : "idle";
  doc["uptime"] = uptime();
  doc["liters"] = station_->session().liters;
  doc["log"] = log_;
  String result;
  serializeJson(doc, result);
  return result;
}

String WebServerController::uptime() const {
  const uint32_t seconds = millis() / 1000;
  return String(seconds / 86400) + "d " + String((seconds / 3600) % 24) + "h " +
         String((seconds / 60) % 60) + "m " + String(seconds % 60) + "s";
}

}  // namespace fuelstation