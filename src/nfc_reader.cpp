#include "nfc_reader.h"

namespace fuelstation {

void NfcReader::begin() {
  nfc_.begin();
  uint32_t version = nfc_.getFirmwareVersion();
  if (!version) {
    Serial.println("PN532 nicht gefunden!");
    while (1) {
      delay(10);
    }
  }
  nfc_.SAMConfig();
  Serial.println("PN532 initialisiert");
}

bool NfcReader::readCardId(String& cardUid) {
  uint8_t uid[8] = {0};
  uint8_t uidLength = 0;

  bool status = nfc_.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000);
  if (!status) {
    return false;
  }

  cardUid = "";
  for (uint8_t i = 0; i < uidLength; ++i) {
    if (uid[i] < 0x10) {
      cardUid += "0";
    }
    cardUid += String(uid[i], HEX);
  }
  cardUid.toUpperCase();
  return true;
}

}  // namespace fuelstation
