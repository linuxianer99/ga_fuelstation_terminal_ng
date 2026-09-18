#include <Arduino.h>

#include "fuel_station_controller.h"

namespace {

fuelstation::FuelStationController station;

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("ESP32-S3 Fuel Station Terminal Start");
  station.begin();
}

void loop() {
  station.update();
  delay(100);
}
