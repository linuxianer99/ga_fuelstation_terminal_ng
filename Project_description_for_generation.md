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