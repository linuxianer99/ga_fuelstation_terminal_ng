#pragma once

#include <Adafruit_PN532.h>

#include "config.h"

namespace fuelstation {

struct RfidCardData {
  String uid;
  String aircraft;
  String articleSuffix;
  String memberId;
  String storedHash;
};

class NfcReader {
 public:
  void begin();
  bool readCard(RfidCardData& card);

 private:
  bool authenticateAndRead(uint8_t* uid, uint8_t uidLength, uint8_t block,
                           uint8_t* data);
  bool readTextBlock(uint8_t* uid, uint8_t uidLength, uint8_t block,
                     String& value);
  bool readHashBlock(uint8_t* uid, uint8_t uidLength, uint8_t block,
                     String& value);

  Adafruit_PN532 nfc_{PIN_PN532_SCK, PIN_PN532_MISO, PIN_PN532_MOSI, PIN_PN532_SS};
};

}  // namespace fuelstation
