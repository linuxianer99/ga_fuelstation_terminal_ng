Die Software soll auf einem ESP32-S3 laufen.
Als Framework soll Arduino kombiniert mit ESP-IDF benutzt werden.
An den ESP32-S3 sind folgende Komponenten angeschlossen:

- PN532 über I2C oder SPI
- Ein HD44780 Display
- Ein Impulszähler der pro 100 Liter einen Impuls ausgibt
- Über WLAN ein HTTPS Backend das mit JSON API angesprochen wird
- Ein Relais zum Schalten der Tankstellenpumpe

Verhalten:
Wird eine NFC Karte mit gelesen, soll auf dem Display eine Meldung ausgegeben werden und die Tankstellenpumpe eingeschaltet werden. Dann werden die Impulse des Impulszählers gezählt. Wird wieder eine NFC Karte erkannt, soll das Relais ausgeschaltet werden und eine JSON Payload für das Backend gesendet werden.

Offline-Mode:
Wenn nach einer Tankung das Backend oder das WLAN nicht verfügbar ist, speichere die Tankung lokal im Filesystem des ESP32 ab. Wenn Backund und WLAN wieder verfügbar sind, sende alle gespeicherten Tankungen zum Backend

Konfiguration:
Die Konfiguration des Tankstellen Terminals soll über eine JSON Datei erfolgen.
Die JSON Datei soll auch mit im Filesystem des ESP32 abgelegt werden. Die JSON Datei soll folgende Parameter haben: 

{
    "ssid": "<SSID here>",
    "wifipassword": "<WIFI key HERE>",
    "buzzerintensity": 20,
    "fuelsort": "<FuelSORT of Pump>",
    "billingserver": "<BILLING Server URL>",
    "terminal_id": "<PUMPID>"
    "pump_timeout": 5000
    "httpuser":"admin",
    "httppassword":"admin",
    "heartbeat":10000,
    "calibration":91
}

Ist beim ersten Start keine JSON Datei vorhanden, sollen Default werden geladen werden und die JSON Datei angelegt werden.

Buzzer:
Ein Buzzer soll mit einem PWM Ausgang des ESP32 angeschlossen sein.

BUZZER_PIN 4
BUZZER_FREQ 2000
BUZZER_LEN 500

Der Buzzer soll einen Ton ausgeben, wenn eine gültige NFC Karte erkannt wird.

Netzwerk:
Es soll ein LAN Chip W5500 integriert werden. Die IP soll per DHCP bezogen werden.
Wann das LAN einen Link hat soll das interne WLAN abgeschaltet werden. Hat das LAN keinen Link, soll das WLAN für die Verbindung benutzt werden.

Backend API Call:
Nach einer erfolgreichen Tankung soll eine JSON Objekt an das Backend geschickt werden. Das JSON Objekt soll folgende Daten enthalten:

{
    "aircraft": "<Aircraft callsign read from NFC Card>",
    "article": "<Sort of fuel (this is configured in the fuel station configuration)>",
    "amount": "<the litres measured during refualing>",
    "date": "<current date and time of the refueling>",
    "memberid": "<the id of the person, read from the NFC card">",
    "auth": "<a hmac hash calcualted of the previous values including a presharded salt>"
}

Webserver:
Der ESP32 soll einen Webserver zur verfügung stellen in dem eine Statusseite angezeigt wird. Auf dieser Statuseite soll der aktuelle Status stehen, der Name der Station, die Uptime. Außerdem soll die config.json herunterladbar sein und eine aktualisiert version hochladbar sein.
Der SerialLog der an dem Serialport des ESP32 ausgegeben wird, soll auch auch auf dieser Webseite angezeigt werden. Es soll die Möglichkeit eines Restart geben.
Weiterhin soll es die Möglichkeit geben, direkt eine neue Firmware hochzuladen, die dann in den ESP32 geschrieben wird.