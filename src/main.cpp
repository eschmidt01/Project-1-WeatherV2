// #include <M5Core2.h>
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>

// ///////////////////////////////////////////////////////////////
// // Variables
// ///////////////////////////////////////////////////////////////
// BLEServer *bleServer;
// BLEService *bleService;
// BLECharacteristic *bleCharacteristic;
// bool deviceConnected = false;
// int timer = 0;

// // See the following for generating UUIDs: https://www.uuidgenerator.net/
// #define SERVICE_UUID "ce062b2f-e42b-4239-b951-f9d4b4abe0ff"
// #define CHARACTERISTIC_UUID "46f27243-ac2d-4b01-b909-4b5711a23a8d"

// ///////////////////////////////////////////////////////////////
// // BLE Server Callback Methods
// ///////////////////////////////////////////////////////////////
// class MyServerCallbacks: public BLEServerCallbacks {
//     void onConnect(BLEServer *pServer) {
//         deviceConnected = true;
//         Serial.println("Device connected...");
//     }
//     void onDisconnect(BLEServer *pServer) {
//         deviceConnected = false;
//         Serial.println("Device disconnected...");
//     }
//     // void onWrite(BLECharacteristic *pCharacteristic) {
//     //     std::string readValue = pCharacteristic->getValue(); // get from standard library
//     //     Serial.printf("The new characteristic value as a STRING is: %s\n", readValue.c_str());
//     //     String valStr = readValue.c_str(); // convert to Arduino String
//     //     int val = valStr.toInt();
//     //     Serial.printf("The new characteristic value as an INT is: %d\n", val);
//     // }
// };

// ///////////////////////////////////////////////////////////////
// // Put your setup code here, to run once
// ///////////////////////////////////////////////////////////////
// void setup()
// {
//     // Init device
//     M5.begin();
//     Serial.print("Starting BLE...");

//     // Initialize M5Core2 as a BLE server
//     BLEDevice::init("Eric's M5Core2");
//     bleServer = BLEDevice::createServer();
//     bleServer->setCallbacks(new MyServerCallbacks());
//     bleService = bleServer->createService(SERVICE_UUID);
//     bleCharacteristic = bleService->createCharacteristic(
//         CHARACTERISTIC_UUID,
//         BLECharacteristic::PROPERTY_READ |
//         BLECharacteristic::PROPERTY_WRITE
//     );
//     bleCharacteristic->setValue("Hello world! Welcome to BLE.");
//     bleService->start();

//     // Start broadcasting (advertising) BLE service
//     BLEAdvertising *bleAdvertising = BLEDevice::getAdvertising(); // getAdvertising() - Static method to grab overall bluetooth capabilities
//     bleAdvertising->addServiceUUID(SERVICE_UUID);
//     // Some general housekeeping
//     bleAdvertising->setScanResponse(true);
//     // Functions that help with iPhone connection issues (connection interval)
//     bleAdvertising->setMinPreferred(0x06);
//     bleAdvertising->setMinPreferred(0x12);
//     BLEDevice::startAdvertising();
//     Serial.println("characteristic defined...you can connect with your phone!");
// }

// ///////////////////////////////////////////////////////////////
// // Put your main code here, to run repeatedly
// ///////////////////////////////////////////////////////////////
// void loop()
// {
//     if (deviceConnected) {
//         // Update the characteristic's value (which can  be read by a client)
//         // timer++;
//         // bleCharacteristic->setValue(timer);
//         // Serial.printf("%d written to BLE characteristic.\n", timer);

//         // Read the characteristic's value as a string (which can be written from a client)
//         std::string readValue = bleCharacteristic->getValue(); // get from standard library
//         Serial.printf("The new characteristic value as a STRING is: %s\n", readValue.c_str());
//         String valStr = readValue.c_str(); // convert to Arduino String
//         int val = valStr.toInt();
//         Serial.printf("The new characteristic value as an INT is: %d\n", val);

//         // Read the characteristic's value as a BYTE (which can be written from a client)
//         // uint8_t *numPtr = new uint8_t(); // create a new pointer to a byte
//         // numPtr = bleCharacteristic->getData();
//         // Serial.printf("The new characteristic value as a BYTE is: %d\n", *numPtr);
//     } else
//         timer = 0;
    
//     // Only update the timer (if connected) every 1 second
//     delay(1000);
// }

#include <M5Unified.h>
#include <Wire.h>
#include "Adafruit_seesaw.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// --- BLE Server Setup ---
#define SERVICE_UUID "ce062b2f-e42b-4239-b951-f9d4b4abe0ff"
#define CHARACTERISTIC_UUID "46f27243-ac2d-4b01-b909-4b5711a23a8d"
bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Device connected...");
  }
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected...");
  }
};

BLEServer *bleServer;
BLEService *bleService;
BLECharacteristic *bleCharacteristic;

// --- Gamepad and Display Setup ---
#define BUTTON_X       6
#define BUTTON_Y       2
#define BUTTON_A       5
#define BUTTON_B       1
#define BUTTON_SELECT  0
#define BUTTON_START   16

uint32_t BUTTON_MASK = (1UL << BUTTON_X) | (1UL << BUTTON_Y) | (1UL << BUTTON_A) |
                       (1UL << BUTTON_B) | (1UL << BUTTON_SELECT) | (1UL << BUTTON_START);
Adafruit_seesaw ss;

int blueX, blueY;
int speedBlue = 1;
bool lastStartPressed = false;
// For random warping, include <stdlib.h> if needed

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  Serial.println("Starting BLE Server with Gamepad...");

  Wire.begin();
  if (!ss.begin(0x50)) {
    Serial.println("Seesaw not found. Check wiring.");
    while (1) delay(10);
  }
  ss.pinModeBulk(BUTTON_MASK, INPUT_PULLUP);

  // Initialize positions
  blueX = 20;
  blueY = 20;

  // --- BLE Initialization ---
  BLEDevice::init("M5Core2_Server");
  bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new MyServerCallbacks());
  bleService = bleServer->createService(SERVICE_UUID);
  bleCharacteristic = bleService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  bleCharacteristic->setValue("Hello from Server!");
  bleService->start();
  BLEAdvertising *bleAdvertising = BLEDevice::getAdvertising();
  bleAdvertising->addServiceUUID(SERVICE_UUID);
  bleAdvertising->setScanResponse(true);
  bleAdvertising->setMinPreferred(0x06);
  bleAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE Advertising started...");
}

void loop() {
  M5.update();

  // If device is connected, you can also exchange data here
  if (deviceConnected) {
    // Example: send the blue dot position as a string over BLE
    char coordStr[20];
    sprintf(coordStr, "%d,%d", blueX, blueY);
    bleCharacteristic->setValue(coordStr);
  }

  // Read joystick values from the seesaw
  int joyX = 1023 - ss.analogRead(14);
  int joyY = 1023 - ss.analogRead(15);
  const int center = 512;
  const int threshold = 50;
  int dx = 0, dy = 0;
  if (joyX > center + threshold) dx = 1;
  else if (joyX < center - threshold) dx = -1;
  if (joyY > center + threshold) dy = -1; // invert for display
  else if (joyY < center - threshold) dy = 1;

  blueX += dx * speedBlue;
  blueY += dy * speedBlue;

  // Speed adjustment using Start button
  uint32_t buttons = ss.digitalReadBulk(BUTTON_MASK);
  bool startPressed = !(buttons & (1UL << BUTTON_START));
  if (startPressed && !lastStartPressed) {
    speedBlue++;
    if (speedBlue > 5) speedBlue = 1;
    Serial.print("Blue speed set to: ");
    Serial.println(speedBlue);
  }
  lastStartPressed = startPressed;

  // Random warp using Select button
  bool selectPressed = !(buttons & (1UL << BUTTON_SELECT));
  if (selectPressed) {
    blueX = random(0, M5.Display.width() - 2);
    blueY = random(0, M5.Display.height() - 2);
    Serial.println("Blue dot warped to a random location!");
  }

  // Constrain dot to screen bounds
  int maxX = M5.Display.width() - 2;
  int maxY = M5.Display.height() - 2;
  if (blueX < 0) blueX = 0;
  if (blueX > maxX) blueX = maxX;
  if (blueY < 0) blueY = 0;
  if (blueY > maxY) blueY = maxY;

  // Refresh display
  M5.Display.fillScreen(BLACK);
  M5.Display.fillRect(blueX, blueY, 2, 2, BLUE);

  delay(20);
}
