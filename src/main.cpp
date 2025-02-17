#include <M5Core2.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "WeatherDisplay.h"
#include "ZipEdit.h"

// WiFi credentials
String wifiNetworkName = "SHaven";
String wifiPassword = "27431sushi";

// OpenWeatherMap API key
String apiKey = "f8ec5beb193c8f444a71879d2a5ecb30";


// Global timing variables
unsigned long lastUpdateTime = 0;
unsigned long timerDelay = 300000; // 5 minutes; adjust for testing if needed

// Application states
enum AppState { WEATHER_SCREEN, ZIP_EDIT_SCREEN };
AppState currentState = WEATHER_SCREEN;

String currentZip = "20001"; // Default zip code
bool isFahrenheit = true;      // Temperature display unit

// NTP client for time synchronization
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

WeatherDisplay weatherDisplay;
ZipEdit zipEdit;

bool needRedraw = true;

void connectWiFi() {
  WiFi.begin(wifiNetworkName.c_str(), wifiPassword.c_str());
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.setRotation(1);
  connectWiFi();
  timeClient.begin();
  
  // Initialize both screens
  weatherDisplay.init(apiKey);
  zipEdit.init(currentZip);

  // Initial weather fetch
  weatherDisplay.updateWeather(currentZip);
  timeClient.update();
  weatherDisplay.lastSyncTime = timeClient.getFormattedTime();
  
  lastUpdateTime = millis();
  needRedraw = true;
}

void loop() {
  M5.update();
  
  switch (currentState) {
    case WEATHER_SCREEN:
      // Update weather every timerDelay
      if (millis() - lastUpdateTime > timerDelay) {
        weatherDisplay.updateWeather(currentZip);
        timeClient.update();
        weatherDisplay.lastSyncTime = timeClient.getFormattedTime();
        lastUpdateTime = millis();
        needRedraw = true;
      }
      
      // Only redraw if needed
      if (needRedraw) {
        weatherDisplay.draw(isFahrenheit);
        needRedraw = false;
      }
      
      // Check for touch on the toggle button
      if (weatherDisplay.checkToggleButtonPressed()) {
        isFahrenheit = !isFahrenheit;
        needRedraw = true;
      }
      
      // Check for touch on the Edit Zip button
      if (weatherDisplay.checkEditZipButtonPressed()) {
        zipEdit.setZip(currentZip);
        currentState = ZIP_EDIT_SCREEN;
        needRedraw = true;
      }
      break;
      
    case ZIP_EDIT_SCREEN:
      // Redraw zip edit screen when needed
      if (needRedraw) {
        zipEdit.display();
        needRedraw = false;
      }
      zipEdit.handleTouch();
      if (zipEdit.isSavePressed()) {
        currentZip = zipEdit.getZip();
        weatherDisplay.updateWeather(currentZip);
        timeClient.update();
        weatherDisplay.lastSyncTime = timeClient.getFormattedTime();
        currentState = WEATHER_SCREEN;
        needRedraw = true;
      }
      break;
  }
  delay(20); // minimal delay to avoid busy-looping
}