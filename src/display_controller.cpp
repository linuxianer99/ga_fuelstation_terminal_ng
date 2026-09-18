#include "display_controller.h"

namespace fuelstation {

void DisplayController::begin() {
  lcd_.init();
  lcd_.backlight();
  lcd_.clear();
  lcd_.setCursor(0, 0);
  lcd_.print("Fuel Station");
  lcd_.setCursor(0, 1);
  lcd_.print("Booting...");
  delay(1000);
  showIdle();
}

void DisplayController::showIdle() {
  lcd_.clear();
  lcd_.setCursor(0, 0);
  lcd_.print("Bereit");
  lcd_.setCursor(0, 1);
  lcd_.print("Karte lesen");
}

void DisplayController::showAccepted(const String& uid) {
  lcd_.clear();
  lcd_.setCursor(0, 0);
  lcd_.print("Karte erkannt");
  lcd_.setCursor(0, 1);
  lcd_.print(uid.substring(0, min(16, uid.length())));
}

void DisplayController::showPumpRunning(float liters) {
  lcd_.clear();
  lcd_.setCursor(0, 0);
  lcd_.print("Pumpe aktiv");
  lcd_.setCursor(0, 1);
  lcd_.print(String(liters, 0));
  lcd_.print(" L");
}

void DisplayController::showPumpStopped() {
  lcd_.clear();
  lcd_.setCursor(0, 0);
  lcd_.print("Stopp");
  lcd_.setCursor(0, 1);
  lcd_.print("Daten senden");
}

}  // namespace fuelstation
