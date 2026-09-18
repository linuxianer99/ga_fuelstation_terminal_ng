#pragma once

#include <Adafruit_PN532.h>

#include "config.h"

namespace fuelstation {

class NfcReader {
 public:
  void begin();
  bool readCardId(String& cardUid);

 private:
  Adafruit_PN532 nfc_{PIN_PN532_SCK, PIN_PN532_MISO, PIN_PN532_MOSI, PIN_PN532_SS};
};

}  // namespace fuelstation
