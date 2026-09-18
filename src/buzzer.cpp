#include "buzzer.h"

#include "esp32-hal-ledc.h"

namespace fuelstation {

void Buzzer::begin(uint8_t intensity) {
  intensity_ = intensity;
  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RESOLUTION_BITS);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);
}

void Buzzer::beep() {
  ledcWrite(BUZZER_CHANNEL, intensity_);
  delay(BUZZER_LEN);
  ledcWrite(BUZZER_CHANNEL, 0);
}

}  // namespace fuelstation
