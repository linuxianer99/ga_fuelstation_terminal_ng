#include "backend_client.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_err.h>
#include <esp_eth.h>
#include <esp_eth_mac.h>
#include <esp_eth_netif_glue.h>
#include <esp_eth_phy.h>
#include <esp_event.h>
#include <esp_mac.h>
#include <esp_netif.h>
#include <mbedtls/md.h>
#include <time.h>

namespace fuelstation {

namespace {

esp_eth_handle_t ethernetHandle = nullptr;
esp_netif_t* ethernetNetif = nullptr;
spi_device_handle_t w5500SpiHandle = nullptr;
volatile bool ethernetLinkUp = false;
volatile bool ethernetHasIp = false;

bool logEthernetError(const char* operation, esp_err_t error) {
  if (error == ESP_OK) {
    return false;
  }
  Serial.print(operation);
  Serial.print(" fehlgeschlagen: ");
  Serial.println(esp_err_to_name(error));
  return true;
}

bool ensureEthernetDhcpClientStarted() {
  if (ethernetNetif == nullptr) {
    Serial.println("W5500-DHCP: Netzwerkschnittstelle fehlt");
    return false;
  }

  esp_netif_dhcp_status_t status = ESP_NETIF_DHCP_STOPPED;
  if (logEthernetError("W5500-DHCP-Status",
                       esp_netif_dhcpc_get_status(ethernetNetif, &status))) {
    return false;
  }
  if (status == ESP_NETIF_DHCP_STARTED) {
    return true;
  }

  const esp_err_t error = esp_netif_dhcpc_start(ethernetNetif);
  if (error == ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED) {
    return true;
  }
  if (logEthernetError("W5500-DHCP-Start", error)) {
    return false;
  }
  Serial.println("W5500-DHCP-Client gestartet");
  return true;
}

void onEthernetEvent(void*, esp_event_base_t, int32_t eventId, void*) {
  if (eventId == ETHERNET_EVENT_CONNECTED) {
    ethernetLinkUp = true;
    Serial.println("W5500-Link aktiv; DHCP-Adresse wird erwartet");
    ensureEthernetDhcpClientStarted();
  } else if (eventId == ETHERNET_EVENT_DISCONNECTED) {
    ethernetLinkUp = false;
    ethernetHasIp = false;
    Serial.println("W5500-Link getrennt");
  }
}

void onEthernetIpEvent(void*, esp_event_base_t, int32_t eventId,
                       void* eventData) {
  if (eventId == IP_EVENT_ETH_GOT_IP) {
    ethernetHasIp = true;
    const auto* event = static_cast<const ip_event_got_ip_t*>(eventData);
    Serial.print("W5500 DHCP-Adresse: ");
    Serial.println(IPAddress(event->ip_info.ip.addr));
  } else if (eventId == IP_EVENT_ETH_LOST_IP) {
    ethernetHasIp = false;
    Serial.println("W5500 DHCP-Adresse verloren");
  }
}

bool beginW5500() {
#if !defined(CONFIG_ETH_SPI_ETHERNET_W5500) || !CONFIG_ETH_SPI_ETHERNET_W5500
  Serial.println("ESP-IDF wurde ohne W5500-SPI-Treiber gebaut");
  return false;
#else
  if (ethernetHandle != nullptr) {
    return true;
  }

  // Initialize the Arduino network stack and event loop before adding ETH.
  if (!WiFi.mode(WIFI_STA)) {
    Serial.println("Netzwerk-Stack konnte nicht initialisiert werden");
    return false;
  }

  pinMode(PIN_W5500_RESET, OUTPUT);
  digitalWrite(PIN_W5500_RESET, HIGH);
  delay(10);
  digitalWrite(PIN_W5500_RESET, LOW);
  delay(50);
  digitalWrite(PIN_W5500_RESET, HIGH);
  delay(100);

  spi_bus_config_t busConfig{};
  busConfig.mosi_io_num = PIN_W5500_MOSI;
  busConfig.miso_io_num = PIN_W5500_MISO;
  busConfig.sclk_io_num = PIN_W5500_SCK;
  busConfig.quadwp_io_num = -1;
  busConfig.quadhd_io_num = -1;
  busConfig.max_transfer_sz = 1600;
  if (logEthernetError("W5500-SPI-Bus", spi_bus_initialize(
                           SPI2_HOST, &busConfig, SPI_DMA_CH_AUTO))) {
    return false;
  }

  spi_device_interface_config_t deviceConfig{};
  deviceConfig.command_bits = 16;
  deviceConfig.address_bits = 8;
  deviceConfig.mode = 0;
  deviceConfig.clock_speed_hz = 20 * 1000 * 1000;
  deviceConfig.spics_io_num = PIN_W5500_CS;
  deviceConfig.queue_size = 20;
  if (logEthernetError("W5500-SPI-Geraet", spi_bus_add_device(
                           SPI2_HOST, &deviceConfig, &w5500SpiHandle))) {
    return false;
  }

  eth_w5500_config_t w5500Config = ETH_W5500_DEFAULT_CONFIG(w5500SpiHandle);
  w5500Config.int_gpio_num = PIN_W5500_INT;
  eth_mac_config_t macConfig = ETH_MAC_DEFAULT_CONFIG();
  esp_eth_mac_t* mac = esp_eth_mac_new_w5500(&w5500Config, &macConfig);
  if (mac == nullptr) {
    Serial.println("W5500-MAC konnte nicht erstellt werden");
    return false;
  }

  eth_phy_config_t phyConfig = ETH_PHY_DEFAULT_CONFIG();
  phyConfig.reset_gpio_num = -1;
  esp_eth_phy_t* phy = esp_eth_phy_new_w5500(&phyConfig);
  if (phy == nullptr) {
    Serial.println("W5500-PHY konnte nicht erstellt werden");
    return false;
  }

  esp_eth_config_t ethernetConfig = ETH_DEFAULT_CONFIG(mac, phy);
  if (logEthernetError("W5500-Treiber", esp_eth_driver_install(
                           &ethernetConfig, &ethernetHandle))) {
    ethernetHandle = nullptr;
    return false;
  }

  uint8_t macAddress[6] = {};
  if (logEthernetError("ESP32-S3 Ethernet-MAC lesen",
                       esp_read_mac(macAddress, ESP_MAC_ETH))) {
    return false;
  }
  if (logEthernetError("W5500-MAC-Adresse setzen",
                       esp_eth_ioctl(ethernetHandle, ETH_CMD_S_MAC_ADDR,
                                     macAddress))) {
    return false;
  }
  uint8_t configuredMacAddress[6] = {};
  if (logEthernetError("W5500-MAC-Adresse lesen",
                       esp_eth_ioctl(ethernetHandle, ETH_CMD_G_MAC_ADDR,
                                     configuredMacAddress))) {
    return false;
  }
  Serial.printf("W5500 MAC-Adresse: %02X:%02X:%02X:%02X:%02X:%02X\n",
                configuredMacAddress[0], configuredMacAddress[1],
                configuredMacAddress[2], configuredMacAddress[3],
                configuredMacAddress[4], configuredMacAddress[5]);

  esp_netif_config_t netifConfig = ESP_NETIF_DEFAULT_ETH();
  ethernetNetif = esp_netif_new(&netifConfig);
  if (ethernetNetif == nullptr) {
    Serial.println("Ethernet-Netzwerkschnittstelle konnte nicht erstellt werden");
    return false;
  }

  esp_eth_netif_glue_handle_t netifGlue = esp_eth_new_netif_glue(ethernetHandle);
  if (netifGlue == nullptr ||
      logEthernetError("Ethernet-Netzwerkschnittstelle",
                       esp_netif_attach(ethernetNetif, netifGlue))) {
    return false;
  }

  if (logEthernetError("Ethernet-Ereignisregistrierung",
                       esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID,
                                                  onEthernetEvent, nullptr)) ||
      logEthernetError("Ethernet-IP-Ereignisregistrierung",
                       esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID,
                                                  onEthernetIpEvent, nullptr))) {
    return false;
  }

  if (logEthernetError("W5500-Start", esp_eth_start(ethernetHandle))) {
    return false;
  }
  Serial.println("W5500-Treiber gestartet; warte auf Ethernet-Link und DHCP");
  return true;
#endif
}

}  // namespace

void BackendClient::begin(const RuntimeConfig& config, bool filesystemReady) {
  config_ = config;
  filesystemReady_ = filesystemReady;
  beginW5500();
}

bool BackendClient::isNetworkConnected() const {
  if (ethernetLinkUp && ethernetHasIp) {
    return true;
  }
  return WiFi.status() == WL_CONNECTED;
}

bool BackendClient::connectWiFi() {
  if (ethernetHandle != nullptr) {
    if (ethernetLinkUp) {
      ensureEthernetDhcpClientStarted();
    }
    for (uint8_t attempt = 0; attempt < 60 && !ethernetHasIp; ++attempt) {
      delay(250);
    }
  }
  if (ethernetLinkUp && ethernetHasIp) {
    WiFi.mode(WIFI_OFF);
    synchronizeClock();
    Serial.print("W5500 per DHCP verbunden: ");
    esp_netif_ip_info_t ipInfo{};
    if (ethernetNetif != nullptr &&
        esp_netif_get_ip_info(ethernetNetif, &ipInfo) == ESP_OK) {
      Serial.println(IPAddress(ipInfo.ip.addr));
    }
    Serial.println("LAN-Link aktiv; internes WiFi deaktiviert");
    return true;
  }
  if (ethernetHandle != nullptr) {
    if (!ethernetLinkUp) {
      Serial.println("W5500: kein Ethernet-Link; Kabel, Switch-Port und Link-LED pruefen");
    } else {
      esp_netif_dhcp_status_t status = ESP_NETIF_DHCP_STOPPED;
      if (!logEthernetError("W5500-DHCP-Status",
                            esp_netif_dhcpc_get_status(ethernetNetif, &status))) {
        Serial.print("W5500: Link aktiv, aber nach 15 Sekunden keine DHCP-Adresse; DHCP-Client ");
        Serial.println(status == ESP_NETIF_DHCP_STARTED ? "laeuft" : "ist gestoppt");
      }
    }
    Serial.println("Verwende WiFi als Netzwerk-Fallback");
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(config_.wifiSsid.c_str(), config_.wifiPassword.c_str());

  Serial.print("Verbinde mit WiFi");
  for (uint8_t i = 0; i < 20 && WiFi.status() != WL_CONNECTED; ++i) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Verbindung fehlgeschlagen");
    return false;
  }

  Serial.println("WiFi verbunden");
  Serial.println(WiFi.localIP());
  synchronizeClock();
  return true;
}

void BackendClient::synchronizeClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  for (uint8_t attempt = 0; attempt < 20 && time(nullptr) < 1700000000; ++attempt) {
    delay(250);
  }
}

bool BackendClient::sendSession(const SessionData& session) {
  if (!isNetworkConnected()) {
    return false;
  }

  return sendPayload(serializeSession(session));
}

bool BackendClient::queueSession(const SessionData& session) {
  if (!filesystemReady_) {
    return false;
  }

  File file = LittleFS.open(nextQueuePath(), FILE_WRITE);
  if (!file) {
    Serial.println("Offline-Session konnte nicht gespeichert werden");
    return false;
  }

  const String payload = serializeSession(session);
  const size_t written = file.print(payload);
  file.close();

  if (written != payload.length()) {
    Serial.println("Offline-Session wurde unvollstaendig gespeichert");
    return false;
  }

  Serial.println("Session fuer Offline-Upload gespeichert");
  return true;
}

void BackendClient::flushPendingSessions() {
  if (!filesystemReady_ || !isNetworkConnected()) {
    return;
  }

  File root = LittleFS.open("/");
  if (!root) {
    return;
  }

  File file = root.openNextFile();
  while (file) {
    const String path = file.name();
    File pending = LittleFS.open(path, FILE_READ);
    String payload;
    if (pending) {
      payload = pending.readString();
      pending.close();
    }
    file.close();

    if (!path.startsWith("/offline_") || payload.isEmpty()) {
      file = root.openNextFile();
      continue;
    }

    if (!sendPayload(payload)) {
      Serial.println("Offline-Upload fehlgeschlagen; Datei bleibt erhalten");
      break;
    }

    LittleFS.remove(path);
    Serial.print("Offline-Session uebertragen: ");
    Serial.println(path);
    file = root.openNextFile();
  }
  root.close();
}

String BackendClient::serializeSession(const SessionData& session) const {
  const String aircraft = session.aircraft;
  const String memberId = session.memberId;
  const String article = session.article;
  const String amount = String(session.liters, 3);
  const String date = currentDateTime();

  JsonDocument doc;
  doc["aircraft"] = aircraft;
  doc["article"] = article;
  doc["amount"] = session.liters;
  doc["date"] = date;
  doc["memberid"] = memberId;
  doc["card_hash"] = session.storedCardHash;
  doc["auth"] = calculateAuth(aircraft, article, amount, date, memberId);

  String payload;
  serializeJson(doc, payload);
  return payload;
}

String BackendClient::currentDateTime() const {
  time_t now = time(nullptr);
  struct tm utcTime;
  gmtime_r(&now, &utcTime);

  char formatted[25] = {0};
  strftime(formatted, sizeof(formatted), "%Y-%m-%dT%H:%M:%SZ", &utcTime);
  return String(formatted);
}

String BackendClient::calculateAuth(const String& aircraft,
                                    const String& article,
                                    const String& amount,
                                    const String& date,
                                    const String& memberId) const {
  const String canonical = aircraft + "|" + article + "|" + amount + "|" +
                           date + "|" + memberId;
  uint8_t digest[32] = {0};
  const mbedtls_md_info_t* mdInfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  mbedtls_md_hmac(mdInfo,
                  reinterpret_cast<const unsigned char*>(config_.hmacSalt.c_str()),
                  config_.hmacSalt.length(),
                  reinterpret_cast<const unsigned char*>(canonical.c_str()),
                  canonical.length(), digest);

  char hexDigest[65] = {0};
  for (uint8_t index = 0; index < sizeof(digest); ++index) {
    snprintf(hexDigest + index * 2, 3, "%02x", digest[index]);
  }
  return String(hexDigest);
}

String BackendClient::nextQueuePath() const {
  String path;
  uint32_t sequence = millis();
  do {
    path = "/offline_" + String(sequence++) + ".json";
  } while (LittleFS.exists(path));
  return path;
}

bool BackendClient::sendPayload(const String& payload) {
  if (!isNetworkConnected()) {
    return false;
  }

  WiFiClientSecure client;
  if (config_.tlsInsecure) {
    client.setInsecure();
  }

  if (!client.connect(config_.apiHost.c_str(), config_.apiPort)) {
    Serial.println("HTTPS Verbindung fehlgeschlagen");
    return false;
  }

  client.print("POST ");
  client.print(config_.apiPath);
  client.print(" HTTP/1.1\r\n");
  client.print("Host: ");
  client.print(config_.apiHost);
  client.print("\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Content-Length: ");
  client.print(payload.length());
  client.print("\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(payload);
  client.print("\r\n");

  const uint32_t timeoutAt = millis() + 5000;
  while (!client.available() && millis() < timeoutAt) {
    delay(10);
  }

  const String statusLine = client.readStringUntil('\n');
  const bool successful = statusLine.startsWith("HTTP/1.1 2") ||
                          statusLine.startsWith("HTTP/1.0 2");
  if (!successful) {
    Serial.print("Backend antwortet mit: ");
    Serial.println(statusLine);
  }
  return successful;
}

}  // namespace fuelstation
