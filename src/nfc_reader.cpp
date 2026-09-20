#include "nfc_reader.h"

namespace fuelstation {

namespace {
constexpr uint8_t MIFARE_KEY_A[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

String bytesToHex(const uint8_t* data, size_t length) {
  String result;
  for (size_t index = 0; index < length; ++index) {
    if (data[index] < 0x10) result += "0";
    result += String(data[index], HEX);
  }
  result.toUpperCase();
  return result;
}
}

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

bool NfcReader::authenticateAndRead(uint8_t* uid, uint8_t uidLength,
                                    uint8_t block, uint8_t* data) {
  if (!nfc_.mifareclassic_AuthenticateBlock(
          uid, uidLength, block, MIFARE_CMD_AUTH_A,
          const_cast<uint8_t*>(MIFARE_KEY_A))) {
    Serial.print("RFID-Authentifizierung fuer Block ");
    Serial.print(block);
    Serial.println(" fehlgeschlagen");
    return false;
  }
  return nfc_.mifareclassic_ReadDataBlock(block, data);
}

bool NfcReader::readTextBlock(uint8_t* uid, uint8_t uidLength, uint8_t block,
                              String& value) {
  uint8_t data[16] = {0};
  if (!authenticateAndRead(uid, uidLength, block, data)) return false;

  uint8_t length = 0;
  while (length < sizeof(data) && data[length] != '\0') ++length;
  if (length == sizeof(data)) {
    Serial.print("RFID-Block ");
    Serial.print(block);
    Serial.println(" ist nicht nullterminiert");
    return false;
  }

  value = String(reinterpret_cast<char*>(data));
  return !value.isEmpty();
}

bool NfcReader::readHashBlock(uint8_t* uid, uint8_t uidLength, uint8_t block,
                              String& value) {
  uint8_t data[16] = {0};
  if (!authenticateAndRead(uid, uidLength, block, data)) return false;
  value += bytesToHex(data, sizeof(data));
  return true;
}

bool NfcReader::readCard(RfidCardData& card) {
  uint8_t uid[8] = {0};
  uint8_t uidLength = 0;

  bool status = nfc_.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000);
  if (!status) {
    return false;
  }

  card = RfidCardData{};
  card.uid = bytesToHex(uid, uidLength);
  if (!readTextBlock(uid, uidLength, 1, card.aircraft)) return false;
  if (!readTextBlock(uid, uidLength, 2, card.articleSuffix)) return false;
  if (!readTextBlock(uid, uidLength, 4, card.memberId)) return false;
  if (!readHashBlock(uid, uidLength, 8, card.storedHash)) return false;
  if (!readHashBlock(uid, uidLength, 9, card.storedHash)) return false;

  Serial.print("RFID gelesen: Aircraft=");
  Serial.print(card.aircraft);
  Serial.print(", Artikel-Suffix=");
  Serial.print(card.articleSuffix);
  Serial.print(", Member-ID=");
  Serial.print(card.memberId);
  Serial.print(", Hash/Zusatzdaten=");
  Serial.println(card.storedHash);
  return true;
}

}  // namespace fuelstation
