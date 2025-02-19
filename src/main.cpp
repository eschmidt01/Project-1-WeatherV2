#include <M5Unified.h>
#include <Wire.h>
#include <Adafruit_VCNL4040.h>

Adafruit_VCNL4040 vcnl;

// Adjusted mapping constants based on observed sensor range.
const uint16_t SENSOR_MIN = 0;   // or 1 if you prefer
const uint16_t SENSOR_MAX = 600;
const int FREQ_MIN = 600;        // Frequency when far
const int FREQ_MAX = 3000;       // Frequency when very close

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);

  // Initialize I2C on pins 27 (SDA) and 19 (SCL)
  if (!Wire.begin(27, 19)) {
    Serial.println("I2C initialization failed!");
    while (1);
  }

  // Initialize the speaker and set volume to max (0-11 per docs)
  M5.Speaker.begin();
  //M5.Speaker.setVolume();

  // Initialize the VCNL4040 sensor.
  if (!vcnl.begin()) {
    Serial.println("VCNL4040 sensor not found!");
    while (1);
  }
  Serial.println("Setup complete.");
}

void loop() {
  M5.update();  // Update internal M5 state (recommended)

  // Read the proximity sensor value.
  uint16_t proximity = vcnl.getProximity();

  // Map the sensor reading to a tone frequency.
  int frequency = map(proximity, SENSOR_MIN, SENSOR_MAX, FREQ_MIN, FREQ_MAX);
  frequency = constrain(frequency, FREQ_MIN, FREQ_MAX);

  // Debug print: shows sensor reading and computed frequency.
  Serial.printf("Proximity: %d, Frequency: %d Hz\n", proximity, frequency);

  // Play the tone at the computed frequency for 200 ms.
  M5.Speaker.tone(frequency, 100); //beep beep

  // Small delay before next reading.
  delay(10*proximity);
}
