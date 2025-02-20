// #include <M5Unified.h>
// #include <Wire.h>
// #include <Adafruit_VCNL4040.h>

// Adafruit_VCNL4040 vcnl;

// // Adjusted mapping constants based on observed sensor range.
// const uint16_t SENSOR_MIN = 0;   // or 1 if you prefer
// const uint16_t SENSOR_MAX = 600;
// const int FREQ_MIN = 600;        // Frequency when far
// const int FREQ_MAX = 3000;       // Frequency when very close

// void setup() {
//   auto cfg = M5.config();
//   M5.begin(cfg);
//   Serial.begin(115200);

//   // Initialize I2C on pins 27 (SDA) and 19 (SCL)
//   if (!Wire.begin(27, 19)) {
//     Serial.println("I2C initialization failed!");
//     while (1);
//   }

//   // Initialize the speaker and set volume to max (0-11 per docs)
//   M5.Speaker.begin();
//   //M5.Speaker.setVolume();

//   // Initialize the VCNL4040 sensor.
//   if (!vcnl.begin()) {
//     Serial.println("VCNL4040 sensor not found!");
//     while (1);
//   }
//   Serial.println("Setup complete.");
// }

// void loop() {
//   M5.update();  // Update internal M5 state (recommended)

//   // Read the proximity sensor value.
//   uint16_t proximity = vcnl.getProximity();

//   // Map the sensor reading to a tone frequency.
//   int frequency = map(proximity, SENSOR_MIN, SENSOR_MAX, FREQ_MIN, FREQ_MAX);
//   frequency = constrain(frequency, FREQ_MIN, FREQ_MAX);

//   // Debug print: shows sensor reading and computed frequency.
//   Serial.printf("Proximity: %d, Frequency: %d Hz\n", proximity, frequency);

//   // Play the tone at the computed frequency for 200 ms.
//   M5.Speaker.tone(frequency, 100); //beep beep

//   // Small delay before next reading.
//   delay(10*proximity);
// }


#include <M5Unified.h>
#include <Wire.h>
#include <Adafruit_VCNL4040.h>

Adafruit_VCNL4040 vcnl;

// ***** Mapping Constants *****
// Sensor readings from VCNL4040 range ~0 (far) to 600 (very close)
const uint16_t SENSOR_MIN = 0;
const uint16_t SENSOR_MAX = 600;

// Tone frequency mapping: 600 Hz when far, up to 3000 Hz when very close
const int TONE_FREQ_MIN = 600;   // Hz
const int TONE_FREQ_MAX = 3000;  // Hz

// Square-wave period for beeping/vibration: longer period when far, shorter when close.
// For example, far = 1000 ms total cycle (500ms on/500ms off), close = 200 ms cycle (100ms on/100ms off)
const unsigned long PERIOD_MAX = 1000; // ms (when far)
const unsigned long PERIOD_MIN = 200;  // ms (when close)

// ***** Timing Variables *****
unsigned long lastToggleTime = 0; // Timestamp of the last toggle
bool beepState = false;           // Current state: true = tone/vibration ON, false = OFF

void setup() {
  // Initialize M5Unified with the default configuration.
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  Serial.println("Starting up...");

  // Initialize I2C on the specified pins (jumper wires on back panel)
  Wire.begin(27, 19);

  // Initialize the speaker.
  M5.Speaker.begin();
  // Optionally, adjust speaker volume here if needed, e.g., M5.Speaker.setVolume(64);

  // Initialize the VCNL4040 sensor.
  if (!vcnl.begin()) {
    Serial.println("VCNL4040 sensor not found!");
    while (1);
  }
  Serial.println("VCNL4040 sensor detected.");

  // (Optional) Identify board before using vibration motor:
  if (M5.getBoard() == m5::board_t::board_M5StackCore2) {
    Serial.println("Board is M5Stack Core2, vibration motor available via AXP_LDO3.");
  } else {
    Serial.println("Board is not M5Stack Core2; check vibration motor support.");
  }

  Serial.println("Setup complete.");
}

void loop() {
  M5.update(); // Update internal state

  // Read the proximity sensor value.
  uint16_t proximity = vcnl.getProximity();

  // Map sensor reading to tone frequency.
  int toneFreq = map(proximity, SENSOR_MIN, SENSOR_MAX, TONE_FREQ_MIN, TONE_FREQ_MAX);
  toneFreq = constrain(toneFreq, TONE_FREQ_MIN, TONE_FREQ_MAX);

  // Map sensor reading to a full square-wave period.
  // When object is far (low reading): period is long.
  // When object is close (high reading): period is short.
  unsigned long period = map(proximity, SENSOR_MIN, SENSOR_MAX, PERIOD_MAX, PERIOD_MIN);
  period = constrain(period, PERIOD_MIN, PERIOD_MAX);
  unsigned long halfPeriod = period / 2;  // 50% duty cycle

  // Get current time.
  unsigned long currentTime = millis();

  // Non-blocking check: if half the computed period has elapsed, toggle the state.
  if (currentTime - lastToggleTime >= halfPeriod) {
    lastToggleTime = currentTime;
    beepState = !beepState;

    if (beepState) {
      // --- TURN ON: Activate tone and vibration ---
      // Play the tone at the computed frequency for halfPeriod milliseconds.
      M5.Speaker.tone(toneFreq, halfPeriod);
      // Activate the vibration motor by setting AXP_LDO3 to 3300 mV.
      // This is specific to the M5Stack Core2.
      if (M5.getBoard() == m5::board_t::board_M5StackCore2) {
        M5.Power.Axp192.setLDO3(3300);
      }
    } else {
      // --- TURN OFF: Stop tone and vibration ---
      M5.Speaker.stop();
      // Turn off the vibration motor.
      if (M5.getBoard() == m5::board_t::board_M5StackCore2) {
        M5.Power.Axp192.setLDO3(0);
      }
    }
  }

  // Optional: Debug output.
  Serial.printf("Proximity: %d, ToneFreq: %d Hz, Period: %lu ms, BeepState: %d\n",
                proximity, toneFreq, period, beepState);

  // A short delay to ease CPU load.
  delay(10);
}
