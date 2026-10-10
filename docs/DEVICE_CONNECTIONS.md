# Geräteanschlüsse

Die verbindliche, vollständige GPIO- und Modul-Pinliste einschließlich
Versorgungs- und Sicherheitshinweisen steht in
[WIRING_OVERVIEW.md](WIRING_OVERVIEW.md).

## Schnellübersicht für den Aufbau

| Gerät / Signal | ESP32-S3 |
|---|---:|
| Pumpenrelais `IN` | GPIO 2 |
| Status-LED-Signal | GPIO 1 |
| Flow-Meter-Impulssignal | GPIO 3 |
| Buzzer-Signal | GPIO 4 |
| LCD-I2C `SDA` | GPIO 8 |
| LCD-I2C `SCL` | GPIO 9 |
| PN532 `SS` / `SSEL` / `CS` (SPI) | GPIO 10 |
| PN532 `MOSI` (SPI) | GPIO 11 |
| PN532 `SCK` (SPI) | GPIO 12 |
| PN532 `MISO` (SPI) | GPIO 13 |

Alle genannten Anschlüsse sind GPIO-Nummern, nicht physische Steckerpositionen.
PN532 ist in der aktuellen Firmware als SPI-Gerät implementiert. Das LCD wird
mit I2C-Adresse `0x27` erwartet.

## W5500 Ethernet

| W5500-Pin | ESP32-S3 |
|---|---:|
| `SCK` | GPIO 14 (separater SPI-Bus) |
| `MISO` / `SO` | GPIO 15 (separater SPI-Bus) |
| `MOSI` / `SI` | GPIO 16 (separater SPI-Bus) |
| `CS` / `SCS` | GPIO 5 |
| `RST` | GPIO 6 |
| `INT` / `IRQ` | GPIO 7 |
| `VCC` | 3V3 bzw. Modul-Datenblatt |
| `GND` | GND |

GPIO 5–7 und 14–16 müssen auf der konkreten Platine herausgeführt und frei sein.
Der PN532 bleibt an GPIO 10–13. Die Firmware initialisiert den W5500 über den
ESP-IDF-SPI-Ethernet-Treiber mit DHCP und verwendet WLAN als Fallback, wenn
Ethernet nicht verbunden ist. Weitere Details stehen in
[WIRING_OVERVIEW.md](WIRING_OVERVIEW.md).

## Hinweise vor dem Einschalten

- ESP32-S3-GPIOs vertragen keine 5-V-Signale. Pegel von Sensor- und
  I2C-Modulausgängen prüfen und nötigenfalls Pegelwandler einsetzen.
- Versorgungsspannungen anhand der Datenblätter der konkreten Module wählen.
- Das Relais muss mit 3,3-V-Logik steuerbar sein; die Pumpenlast gehört in
  einen passend abgesicherten und fachgerecht ausgelegten Lastkreis.
- Die mitgelieferte Flow-Meter-Kalibrierung lautet `pulse_per_litre: 90`.
  Kalibrierwert zum realen Sensor passend konfigurieren und vor Pumpenbetrieb
  prüfen.
