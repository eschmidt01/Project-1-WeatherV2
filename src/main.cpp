#include <M5Unified.h>
#include <Wire.h>
#include <Adafruit_VCNL4040.h>

Adafruit_VCNL4040 vcnl;

const uint16_t SENSOR_MIN = 0, SENSOR_MAX = 600;
const int TONE_FREQ_MIN = 600, TONE_FREQ_MAX = 3000;
const unsigned long PERIOD_MAX = 1000, PERIOD_MIN = 200;

unsigned long lastToggleTime = 0;
bool beepState = false;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  Serial.println("Starting up...");
  Wire.begin(27, 19);
  M5.Speaker.begin();

  if (!vcnl.begin()) {
    Serial.println("VCNL4040 sensor not found!");
    while (1);
  }
  Serial.println("VCNL4040 sensor detected.");

  if (M5.getBoard() == m5::board_t::board_M5StackCore2)
    Serial.println("Board is M5Stack Core2, vibration motor available.");
  else
    Serial.println("Board is not M5Stack Core2; check vibration motor support.");

  Serial.println("Setup complete.");
}

void loop() {
  M5.update();
  uint16_t proximity = vcnl.getProximity();
  int toneFreq = constrain(map(proximity, SENSOR_MIN, SENSOR_MAX, TONE_FREQ_MIN, TONE_FREQ_MAX), TONE_FREQ_MIN, TONE_FREQ_MAX);
  unsigned long period = constrain(map(proximity, SENSOR_MIN, SENSOR_MAX, PERIOD_MAX, PERIOD_MIN), PERIOD_MIN, PERIOD_MAX);
  unsigned long halfPeriod = period / 2;
  unsigned long currentTime = millis();

  if (currentTime - lastToggleTime >= halfPeriod) {
    lastToggleTime = currentTime;
    beepState = !beepState;
    if (beepState) {
      M5.Speaker.tone(toneFreq, halfPeriod);
      if (M5.getBoard() == m5::board_t::board_M5StackCore2)
        M5.Power.Axp192.setLDO3(3300);
    } else {
      M5.Speaker.stop();
      if (M5.getBoard() == m5::board_t::board_M5StackCore2)
        M5.Power.Axp192.setLDO3(0);
    }
  }

  Serial.printf("Proximity: %d, ToneFreq: %d Hz, Period: %lu ms, BeepState: %d\n",
                proximity, toneFreq, period, beepState);
  delay(10);
}