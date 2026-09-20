#include "status_led.h"

#include "esp32-hal-ledc.h"

namespace fuelstation {

void StatusLed::begin() {
  ledcSetup(STATUS_LED_CHANNEL, STATUS_LED_FREQ, STATUS_LED_RESOLUTION_BITS);
  ledcAttachPin(PIN_STATUS_LED, STATUS_LED_CHANNEL);
  ledcWrite(STATUS_LED_CHANNEL, 0);
  fadeStartedAt_ = millis();
}

void StatusLed::update(StationState state) {
  if (state != StationState::Idle) {
    ledcWrite(STATUS_LED_CHANNEL, 0);
    return;
  }

  constexpr uint32_t fadeDurationMs = 2000;
  constexpr uint32_t cycleDurationMs = fadeDurationMs * 2;
  const uint32_t phase = (millis() - fadeStartedAt_) % cycleDurationMs;
  const uint32_t brightness = phase <= fadeDurationMs
                                  ? phase * 255 / fadeDurationMs
                                  : (cycleDurationMs - phase) * 255 / fadeDurationMs;
  ledcWrite(STATUS_LED_CHANNEL, brightness);
}

}  // namespace fuelstation