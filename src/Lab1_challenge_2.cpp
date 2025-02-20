#include <M5Unified.h>
#include <Wire.h>
#include "Adafruit_seesaw.h"

// --- Gamepad Seesaw Definitions ---
#define BUTTON_X       6    // Top button (used for moving red dot up)
#define BUTTON_Y       2    // Left button (red dot left)
#define BUTTON_A       5    // Right button (red dot right)
#define BUTTON_B       1    // Bottom button (red dot down)
#define BUTTON_SELECT  0    // Increases red dot speed
#define BUTTON_START   16   // Increases blue dot speed

// Create a button mask for all buttons of interest.
uint32_t BUTTON_MASK = (1UL << BUTTON_X) | (1UL << BUTTON_Y) | (1UL << BUTTON_A) |
                       (1UL << BUTTON_B) | (1UL << BUTTON_SELECT) | (1UL << BUTTON_START);

// Create an instance for the seesaw gamepad.
Adafruit_seesaw ss;

// --- Global Variables for Dot Positions & Speeds ---
int blueX, blueY;  // Blue dot controlled by the joystick.
int redX, redY;    // Red dot controlled by the D-pad buttons.
int speedBlue = 1; // Movement increment for blue dot (controlled by Start button).
int speedRed = 1;  // Movement increment for red dot (controlled by Select button).

// Variables to help debounce speed-change buttons.
bool lastStartPressed = false;
bool lastSelectPressed = false;

// Game timing and state.
unsigned long gameStartTime = 0;
bool gameOver = false;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  Serial.println("Starting Gamepad QT Challenge #2...");

  // Initialize I2C for the gamepad (the STEMMA connector uses I2C).
  Wire.begin();

  // Initialize the seesaw (default I2C address 0x50 for the Gamepad QT).
  if (!ss.begin(0x50)) {
    Serial.println("Seesaw not found. Check Gamepad wiring.");
    while (1) delay(10);
  }
  // Set up all button pins with internal pullups.
  ss.pinModeBulk(BUTTON_MASK, INPUT_PULLUP);

  // Initialize dot positions:
  // Place blue dot toward the upper–left and red dot toward the lower–right.
  blueX = 20;
  blueY = 20;
  redX = M5.Display.width() - 20;
  redY = M5.Display.height() - 20;

  // Record game start time.
  gameStartTime = millis();
}

void loop() {
  M5.update();

  // If the game is over, do nothing further.
  if (gameOver) return;

  // --- Read Joystick for Blue Dot Movement ---
  // The Gamepad QT’s joystick is read from seesaw analog pins 14 (x-axis) and 15 (y-axis).
  // Per documentation, subtract the read value from 1023 to orient it properly.
  int joyX = 1023 - ss.analogRead(14);
  int joyY = 1023 - ss.analogRead(15);
  const int center = 512;
  const int threshold = 50;  // Dead zone to avoid jitter

  int dx = 0, dy = 0;
  if (joyX > center + threshold) dx = 1;
  else if (joyX < center - threshold) dx = -1;
  if (joyY > center + threshold) dy = -1;  // Flip up/down (invert)
  else if (joyY < center - threshold) dy = 1;


  // Move blue dot according to joystick direction multiplied by its speed.
  blueX += dx * speedBlue;
  blueY += dy * speedBlue;

  // --- Read Button Presses for Red Dot Movement and Speed Adjustments ---
  // Get the current button state (active low; pressed buttons will have a 0 bit).
  uint32_t buttons = ss.digitalReadBulk(BUTTON_MASK);

  // Red dot movement (D-pad):
  // Button X (pin 6) → Move up.
  if (!(buttons & (1UL << BUTTON_X))) {
    redY -= speedRed;
  }
  // Button B (pin 1) → Move down.
  if (!(buttons & (1UL << BUTTON_B))) {
    redY += speedRed;
  }
  // Button Y (pin 2) → Move left.
  if (!(buttons & (1UL << BUTTON_Y))) {
    redX -= speedRed;
  }
  // Button A (pin 5) → Move right.
  if (!(buttons & (1UL << BUTTON_A))) {
    redX += speedRed;
  }

  // Speed adjustments:
  // Start button increases blue dot speed.
  bool startPressed = !(buttons & (1UL << BUTTON_START));
  if (startPressed && !lastStartPressed) {
    speedBlue++;
    if (speedBlue > 5) speedBlue = 1;
    Serial.print("Blue speed set to: ");
    Serial.println(speedBlue);
  }
  // Select button increases red dot speed.
  bool selectPressed = !(buttons & (1UL << BUTTON_SELECT));
  if (selectPressed && !lastSelectPressed) {
    speedRed++;
    if (speedRed > 5) speedRed = 1;
    Serial.print("Red speed set to: ");
    Serial.println(speedRed);
  }
  lastStartPressed = startPressed;
  lastSelectPressed = selectPressed;

  // --- Boundary Check ---
  int maxX = M5.Display.width() - 2;   // dot width: 2 pixels
  int maxY = M5.Display.height() - 2;  // dot height: 2 pixels

  if (blueX < 0) blueX = 0;
  if (blueX > maxX) blueX = maxX;
  if (blueY < 0) blueY = 0;
  if (blueY > maxY) blueY = maxY;

  if (redX < 0) redX = 0;
  if (redX > maxX) redX = maxX;
  if (redY < 0) redY = 0;
  if (redY > maxY) redY = maxY;

  // --- Draw the Game ---
  // Clear the screen.
  M5.Display.fillScreen(BLACK);
  // Draw the blue dot (2x2 pixel) in BLUE.
  M5.Display.fillRect(blueX, blueY, 2, 2, BLUE);
  // Draw the red dot (2x2 pixel) in RED.
  M5.Display.fillRect(redX, redY, 2, 2, RED);

  // --- Collision Detection ---
  // If the two dots are within 5 pixels (Euclidean distance) of each other, end the game.
  int diffX = blueX - redX;
  int diffY = blueY - redY;
  int distSq = diffX * diffX + diffY * diffY;
  const int collisionThresholdSq = 5 * 5;
  if (distSq < collisionThresholdSq) {
    gameOver = true;
    unsigned long currentTime = millis();
    float elapsedTime = (currentTime - gameStartTime) / 1000.0;  // in seconds

    // Clear screen and display "GAME OVER" with elapsed time.
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(20, M5.Display.height() / 2 - 20);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.print("GAME OVER");
    M5.Display.setCursor(20, M5.Display.height() / 2 + 10);
    M5.Display.print("Time: ");
    M5.Display.print(elapsedTime, 2);
    M5.Display.print(" s");

    // Halt the game loop.
    while (1) {
      M5.update();
      delay(100);
    }
  }

  delay(20);  // Small delay to control update rate.
}
