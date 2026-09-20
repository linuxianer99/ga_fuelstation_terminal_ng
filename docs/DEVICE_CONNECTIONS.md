# Geräte-Anschlussübersicht am ESP32

## 1. Überblick

Dieses Projekt verbindet den ESP32-S3 mit folgenden externen Geräten:

- PN532 NFC-Reader
- HD44780 / I2C-LCD
- Impulszähler / Flow Meter
- Relais für die Pumpe
- Buzzer
- optional: W5500 Ethernet-Modul für LAN

Die Pin-Definitionen stehen in [include/config.h](../include/config.h).

## 2. Aktuell definierte Hardware-Pins

| Funktion | Gerät | ESP32-Pin |
|---|---|---:|
| Pumpenrelais | Relais | GPIO 2 |
| Status-LED | LED/PWM | GPIO 1 |
| Flow Meter Input | Impulszähler | GPIO 3 |
| PN532 SCK | NFC | GPIO 12 |
| PN532 MISO | NFC | GPIO 13 |
| PN532 MOSI | NFC | GPIO 11 |
| PN532 SS / CS | NFC | GPIO 10 |
| Buzzer | PWM-Ausgang | GPIO 4 |
| LCD SDA | I2C | GPIO 8 |
| LCD SCL | I2C | GPIO 9 |

## 3. Anschlussliste

### PN532 NFC-Reader (SPI)

- SCK → GPIO 12
- MISO → GPIO 13
- MOSI → GPIO 11
- SS / CS → GPIO 10
- VCC → 3.3V
- GND → GND

### Relais für Tankstellenpumpe

- Signal / IN → GPIO 2
- VCC / Steuerstrom → je nach Relaismodul
- GND → GND

### Impulszähler / Flow Meter

- Signal → GPIO 3
- GND → GND
- je nach Sensor auch Versorgung 3.3V oder 5V

### Buzzer

- Signal → GPIO 4
- GND → GND

### Status-LED

- Anode bzw. Signal → GPIO 1 (mit geeignetem Vorwiderstand)
- Kathode → GND
- Im Idle-Zustand wird die LED innerhalb von 2 Sekunden heller und innerhalb weiterer 2 Sekunden dunkler.

### LCD 16x2 I2C

- SDA → GPIO 8
- SCL → GPIO 9
- VCC → je nach Modul 3.3V oder 5V
- GND → GND

### W5500 Ethernet (optional)

Das Projekt beschreibt zusätzlich ein W5500-LAN-Modul mit DHCP-Adresszuweisung.

Eine praxisnahe und gut nutzbare Pinbelegung auf einem separaten SPI-Bus ist:

- SCK → GPIO 12
- MISO → GPIO 13
- MOSI → GPIO 11
- CS → GPIO 10
- RST → GPIO 9
- INT / IRQ → GPIO 8
- VCC → 3.3V
- GND → GND

Diese Zuordnung vermeidet die bereits belegten PN532- und Buzzer-Pins und lässt die bestehende NFC-Verkabelung unverändert. Der W5500 kann damit auf einem zweiten SPI-Bus oder einem zusätzlichen, freien SPI-Port betrieben werden, ohne die vorhandenen PN532-Pins zu überschreiben.

Hinweis: In der aktuellen Firmware sind die W5500-Pins noch nicht hart codiert; sie müssen je nach Hardware-Layout und der späteren SPI-Initialisierung konfiguriert werden.

## 4. Wichtigste Hinweise

- Das Buzzer-Pin 4 ist laut Beschreibung auch als Flow-Meter-Pin reserviert; in der Praxis müssen diese Signale physisch getrennt sein, damit keine Signalüberlagerung entsteht.
- Der LAN-Link hat Vorrang: Wenn Ethernet aktiv ist, soll das interne WiFi abgeschaltet werden.
- Wenn kein Ethernet-Link vorhanden ist, verwendet das System WiFi, um die Session an das Backend zu senden.

## 5. Kurzform im Schaltplanstil

```text
ESP32-S3
  ├─ GPIO 33 ──> Relais ──> Pumpe
  ├─ GPIO 3  ──> Flow Meter / Impulszähler
  ├─ GPIO 36 ──> PN532 SCK
  ├─ GPIO 35 ──> PN532 MISO
  ├─ GPIO 37 ──> PN532 MOSI
  ├─ GPIO 7  ──> PN532 SS / CS
  ├─ GPIO 4  ──> Buzzer
  ├─ I2C SDA/SCL ──> LCD I2C
  └─ SPI + CS/RST/IRQ ──> W5500 Ethernet (optional)
```
