# Exakte Anschluss- und Pinliste

Diese Belegung entspricht der aktuellen Firmware für das benutzerdefinierte
ESP32-S3-Board. Die Werte sind **GPIO-Nummern**, keine Positionen auf einem
Stecker. Für die physische Position ist der Bestückungs- bzw. Board-Schaltplan
maßgeblich.

## Fest verdrahtete GPIOs

| ESP32-S3 GPIO | Signalrichtung am ESP32 | Anschluss / Modul-Pin | Firmware-Bezeichner |
|---:|---|---|---|
| GPIO 1 | Ausgang (PWM) | Status-LED-Signal, Vorwiderstand verwenden | `PIN_STATUS_LED` |
| GPIO 2 | Ausgang | Relaismodul `IN` / `SIG` für die Pumpe | `PIN_RELAY` |
| GPIO 3 | Eingang, `INPUT_PULLUP`, Interrupt auf steigender Flanke | Flow Meter `OUT` / Impulssignal | `PIN_FLOW_METER` |
| GPIO 4 | Ausgang (PWM) | Buzzer `SIG` / `+` (abhängig vom Buzzer-Modul) | `BUZZER_PIN` |
| GPIO 8 | I2C `SDA` | LCD-I2C `SDA` | `PIN_I2C_SDA` |
| GPIO 9 | I2C `SCL` | LCD-I2C `SCL` | `PIN_I2C_SCL` |
| GPIO 10 | SPI-Ausgang / Chip Select | PN532 `SS` / `SSEL` / `CS` | `PIN_PN532_SS` |
| GPIO 11 | SPI-Ausgang | PN532 `MOSI` | `PIN_PN532_MOSI` |
| GPIO 12 | SPI-Takt | PN532 `SCK` | `PIN_PN532_SCK` |
| GPIO 13 | SPI-Eingang | PN532 `MISO` | `PIN_PN532_MISO` |

## Modul-zu-Modul-Verdrahtung

### PN532 im SPI-Modus

| PN532-Pin | ESP32-S3 |
|---|---|
| `SCK` | GPIO 12 |
| `MISO` | GPIO 13 |
| `MOSI` | GPIO 11 |
| `SS` / `SSEL` / `CS` | GPIO 10 |
| `VCC` | 3V3, sofern das PN532-Modul 3,3 V unterstützt |
| `GND` | GND |

Den PN532-Schalter bzw. die Jumper ausdrücklich auf **SPI** stellen. Die
Firmware instanziiert den Reader mit den vier SPI-GPIOs; I2C-Betrieb des PN532
ist damit nicht die implementierte Anschlussvariante.

### HD44780-kompatibles LCD mit I2C-Backpack

| LCD-/Backpack-Pin | ESP32-S3 |
|---|---|
| `SDA` | GPIO 8 |
| `SCL` | GPIO 9 |
| `VCC` | Versorgung passend zum konkreten Backpack |
| `GND` | GND |

Die Firmware erwartet die I2C-Adresse `0x27`. Vor dem Einbau prüfen, ob das
Backpack diese Adresse verwendet. Bei einem mit 5 V versorgten Backpack
sicherstellen, dass dessen I2C-Pull-ups die ESP32-S3-Leitungen nicht auf 5 V
ziehen; andernfalls Pegelwandler verwenden.

### Durchflussmesser / Impulsgeber

| Sensor-Pin | ESP32-S3 |
|---|---|
| `OUT` / Impulssignal | GPIO 3 |
| `GND` | GND |
| `VCC` | Laut Datenblatt des Sensors; Signalausgang muss ESP32-tauglich sein |

Der Eingang ist mit internem Pull-up konfiguriert und zählt Interrupts an der
steigenden Flanke. Ein 5-V-Ausgang darf nicht direkt an GPIO 3 angeschlossen
werden; bei Bedarf Pegelanpassung vorsehen. Die Firmware berechnet Liter als
`Impulszahl / pulse_per_litre`. Im mitgelieferten `data/config.json` ist
`pulse_per_litre` auf `90` gesetzt. Für einen Sensor mit **1 Impuls je 100 l**
wäre rechnerisch `pulse_per_litre: 0.01` einzutragen. Sensor-Kalibrierung und
Zählverhalten vor Pumpenbetrieb sicher verifizieren.

### Pumpenrelais

| Relaismodul-Pin | ESP32-S3 / Versorgung |
|---|---|
| `IN` / `SIG` | GPIO 2 |
| `VCC` | Nach Vorgabe des Relaismoduls |
| `GND` | GND des Steuersignals |

Das Relaismodul muss mit einem 3,3-V-GPIO-Signal sicher schalten. Die
GPIO-Belegung beschreibt nur die **Steuerseite**; sie ist keine Freigabe, eine
Pumpe direkt am ESP32 anzuschließen. Lastkreis, Absicherung, galvanische
Trennung und Not-Aus fachgerecht auslegen. Die Firmware setzt GPIO 2 beim
Start auf LOW und schaltet beim Tankstart auf HIGH; aktive-Low-Relaismodule
benötigen eine geeignete Anpassung.

### Buzzer

| Buzzer-Pin | ESP32-S3 |
|---|---|
| `SIG` / Steuereingang | GPIO 4 |
| `GND` / `-` | GND |

Bei einem passiven Buzzer wird PWM verwendet (2 kHz). Einen Buzzer, der mehr
Strom benötigt als ein GPIO liefern darf, über einen geeigneten Treiber
anschließen.

### Status-LED

| LED-Anschluss | ESP32-S3 |
|---|---|
| Anode über passenden Vorwiderstand | GPIO 1 |
| Kathode | GND |

Die LED wird per PWM angesteuert und pulsiert im Idle-Zustand. Eine externe LED
nur dann anschließen, wenn GPIO 1 auf dem verwendeten Board dafür verfügbar ist.

## Versorgung und gemeinsame Masse

- Die ESP32-S3-GPIOs sind **nicht 5-V-tolerant**. Kein GPIO-Signal mit 5 V
  beaufschlagen.
- Alle verbundenen Module mit gemeinsamer Signalmasse benötigen eine
  gemeinsame GND-Referenz zum ESP32. Galvanisch getrennte Relais- bzw.
  Leistungskreise entsprechend deren Schaltungsdesign behandeln.
- Modul-Versorgung immer anhand des konkreten Datenblatts festlegen; die
  GPIO-Liste legt keine Stromversorgung oder Belastbarkeit fest.
- Vor dem Einschalten auf Kurzschlüsse, Verpolung und externe Pull-ups prüfen.

## W5500 am separaten SPI-Bus

Der W5500 erhält einen eigenen SPI-Bus, damit er nicht mit dem Software-SPI des
PN532 kollidiert. Die Firmware verwendet folgende Zuordnung:

| W5500-Pin | ESP32-S3 | Hinweis |
|---|---|---|
| `SCK` | GPIO 14 | Separater SPI-Takt |
| `MISO` / `SO` | GPIO 15 | Separate Datenleitung vom W5500 zum ESP32 |
| `MOSI` / `SI` | GPIO 16 | Separate Datenleitung vom ESP32 zum W5500 |
| `SCS` / `CS` | GPIO 5 | W5500-Chip-Select |
| `RST` / `RESET` | GPIO 6 | W5500-Reset, typischerweise aktiv LOW |
| `INT` / `IRQ` | GPIO 7 | W5500-Interrupt |
| `VCC` | 3V3 bzw. laut Modul-Datenblatt | GPIO-Signale des W5500 müssen 3,3-V-kompatibel sein |
| `GND` | GND | Gemeinsame Signalmasse |

Diese Pins müssen auf der konkreten Platine herausgeführt und frei sein.
GPIO 8 bleibt ausschließlich LCD-SDA und GPIO 9 ausschließlich LCD-SCL.
Der PN532 bleibt auf seinem bisherigen Software-SPI-Bus (GPIO 10–13).

Die Firmware initialisiert den W5500 über den ESP-IDF-SPI-Ethernet-Treiber und
bezieht die Adresse per DHCP. Bei vorhandenem LAN-Link und erhaltener DHCP-
Adresse wird WiFi abgeschaltet; andernfalls wird WiFi als Fallback verwendet.

## Firmware-Referenz

Die Pin-Konstanten sind in [`include/config.h`](../include/config.h) definiert.
Die I2C- und SPI-Initialisierungen befinden sich in
[`src/display_controller.cpp`](../src/display_controller.cpp) und
[`include/nfc_reader.h`](../include/nfc_reader.h).
