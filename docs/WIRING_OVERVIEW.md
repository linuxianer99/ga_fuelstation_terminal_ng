# Schaltplan-Übersicht

## 1. Grundlayout

```text
                     +----------------------------------+
                     |           ESP32-S3               |
                     |                                  |
                     |  GPIO 33  ────────> Relais      |
                     |  GPIO 3   ────────> Flow Meter  |
                     |  GPIO 36  ────────> PN532 SCK   |
                     |  GPIO 35  ────────> PN532 MISO  |
                     |  GPIO 37  ────────> PN532 MOSI  |
                     |  GPIO 7   ────────> PN532 SS    |
                     |  GPIO 4   ────────> Buzzer      |
                     |  I2C SDA/SCL ─────> LCD        |
                     |  SPI SCK/MISO/MOSI ─> W5500    |
                     +----------------------------------+
                               |                |
        +----------------------+
        |                       |
        v                       v
  +----------+          +----------------------+
  | Relais   |          | Tankstellenpumpe     |
  +----------+          +----------------------+

  +----------------+    +--------------------------+
  | PN532 NFC      |    | LCD 16x2 I2C            |
  | Reader         |    | Display                 |
  +----------------+    +--------------------------+

  +--------------------+  +----------------------------+
  | Flow Meter /       |  | Buzzer                    |
  | Impulszähler        |  |                          |
  +--------------------+  +----------------------------+

  +----------------------------+
  | W5500 Ethernet Module      |
  | (optional, DHCP enabled)   |
  +----------------------------+
```

## 2. Signal-Übersicht

```text
ESP32                                    Externe Geräte
-----------------------------------------------------------------------
GPIO 33  ------------------------------> Relais / Pump Control
GPIO 3   ------------------------------> Flow Meter pulse input
GPIO 36  ------------------------------> PN532 SCK
GPIO 35  ------------------------------> PN532 MISO
GPIO 37  ------------------------------> PN532 MOSI
GPIO 7   ------------------------------> PN532 SS / CS
GPIO 4   ------------------------------> Buzzer
I2C SDA/SCL ---------------------------> LCD I2C
SPI SCK/MISO/MOSI + CS/RST/IRQ -------> W5500 Ethernet (optional)
```

## 3. Empfohlene W5500-Pinbelegung

Für ein zusätzliches W5500-Ethernetmodul ist eine typische, gut getrennte Verdrahtung:

```text
W5500 Ethernet Module
  ├─ SCK  -> GPIO 12
  ├─ MISO -> GPIO 13
  ├─ MOSI -> GPIO 11
  ├─ CS   -> GPIO 10
  ├─ RST  -> GPIO 9
  ├─ INT  -> GPIO 8
  ├─ VCC  -> 3.3V
  └─ GND  -> GND
```

Diese Pins bleiben frei und vermeiden Kollisionen mit den bereits reservierten Signalen für PN532, Buzzer und Relais.

## 4. Hinweise

- Die Projektbeschreibung erwartet ein W5500 Ethernet-Modul zusätzlich zum internen WiFi.
- Ethernet hat Priorität: Wenn der LAN-Link aktiv ist, soll das interne WiFi abgeschaltet werden.
- Wenn kein LAN-Link vorhanden ist, meldet das System den Zustand als WiFi-Only und versucht die Verbindung über das interne WiFi.
- Die Pinbelegung für W5500 ist im aktuellen Firmware-Stand noch nicht fest codiert und muss je nach Schaltungsdesign ergänzt werden.
