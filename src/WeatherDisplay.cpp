#include "WeatherDisplay.h"
#include "EGR425_Phase1_weather_bitmap_images.h"  // Contains bitmap arrays and getWeatherBitmap()
#include <WiFi.h>

WeatherDisplay::WeatherDisplay() {
    cityName = "";
    strWeatherDesc = "";
    strWeatherIcon = "";
    tempNow = 0.0;
    tempMin = 0.0;
    tempMax = 0.0;
    lastSyncTime = "";
}

void WeatherDisplay::init(String _apiKey) {
    apiKey = _apiKey;
    M5.Lcd.fillScreen(BLACK);
}

String WeatherDisplay::httpGETRequest(const char* serverURL) {
    HTTPClient http;
    http.begin(serverURL);
    int httpResponseCode = http.GET();
    String payload = "";
    if (httpResponseCode > 0) {
        payload = http.getString();
    } else {
        Serial.printf("HTTP GET error: %d\n", httpResponseCode);
    }
    http.end();
    return payload;
}

void WeatherDisplay::updateWeather(String zip) {
    // Build the URL using the zip-code API (country hardcoded to "us")
    String serverURL = urlOpenWeather + "zip=" + zip + ",us&units=imperial&appid=" + apiKey;
    String response = httpGETRequest(serverURL.c_str());
    
    const size_t capacity = 1024;
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return;
    }
    
    JsonArray arrWeather = doc["weather"];
    if (arrWeather.size() > 0) {
        JsonObject objWeather0 = arrWeather[0];
        strWeatherDesc = objWeather0["main"].as<String>();
        strWeatherIcon = objWeather0["icon"].as<String>();
    }
    cityName = doc["name"].as<String>();
    
    JsonObject objMain = doc["main"];
    tempNow = objMain["temp"].as<double>();
    tempMin = objMain["temp_min"].as<double>();
    tempMax = objMain["temp_max"].as<double>();
    
    Serial.printf("City: %s, Temp: %.1f, Low: %.1f, High: %.1f, Desc: %s\n",
                  cityName.c_str(), tempNow, tempMin, tempMax, strWeatherDesc.c_str());
}

void WeatherDisplay::draw(bool isFahrenheit) {
    // Set background color based on day ('d') or night ('n') icon
    uint16_t primaryTextColor;
    if (strWeatherIcon.indexOf("d") >= 0) {
        M5.Lcd.fillScreen(TFT_CYAN);
        primaryTextColor = TFT_DARKGREY;
    } else {
        M5.Lcd.fillScreen(TFT_NAVY);
        primaryTextColor = TFT_WHITE;
    }
    
    // Draw weather icon on the right
    drawWeatherImage(strWeatherIcon, 3);
    
    int pad = 10;
    // Draw low temperature
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(pad, pad);
    M5.Lcd.setTextColor(TFT_BLUE);
    M5.Lcd.printf("Low: %.0f%s\n", isFahrenheit ? tempMin : (tempMin - 32) * 5.0/9.0, isFahrenheit ? "F" : "C");
    
    // Draw current temperature in large font
    M5.Lcd.setCursor(pad, M5.Lcd.getCursorY());
    M5.Lcd.setTextSize(5);
    M5.Lcd.setTextColor(primaryTextColor);
    double displayTemp = isFahrenheit ? tempNow : (tempNow - 32) * 5.0/9.0;
    M5.Lcd.printf("%.0f%s\n", displayTemp, isFahrenheit ? "F" : "C");
    
    // Draw high temperature
    M5.Lcd.setCursor(pad, M5.Lcd.getCursorY());
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.printf("High: %.0f%s\n", isFahrenheit ? tempMax : (tempMax - 32) * 5.0/9.0, isFahrenheit ? "F" : "C");
    
    // Draw city name
    M5.Lcd.setCursor(pad, M5.Lcd.getCursorY());
    M5.Lcd.setTextColor(primaryTextColor);
    M5.Lcd.printf("%s\n", cityName.c_str());
    
    // Draw last sync timestamp
    M5.Lcd.setCursor(pad, M5.Lcd.getCursorY());
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Last: %s\n", lastSyncTime.c_str());
    
    // Draw F/C toggle button (left bottom) with adjusted size and text
    M5.Lcd.drawRect(pad, M5.Lcd.height() - 60, 60, 40, TFT_WHITE);
    M5.Lcd.setCursor(pad + 15, M5.Lcd.height() - 40);
    M5.Lcd.print("F/C");
    
    // Draw Edit Zip button (right bottom)
    M5.Lcd.drawRect(M5.Lcd.width() - 150, M5.Lcd.height() - 60, 140, 40, TFT_WHITE);
    M5.Lcd.setCursor(M5.Lcd.width() - 140, M5.Lcd.height() - 40);
    M5.Lcd.print("Edit Zip");
}

bool WeatherDisplay::checkToggleButtonPressed() {
    static unsigned long lastToggleTime = 0;
    if (M5.Touch.ispressed()) {
        auto pos = M5.Touch.getPressPoint();
        int x = pos.x;
        int y = pos.y;
        // Left-bottom button area
        if (x >= 10 && x <= 70 && y >= M5.Lcd.height() - 60 && y <= M5.Lcd.height() - 20) {
            // Debounce the button press with a 300ms delay
            if (millis() - lastToggleTime > 300) {
                lastToggleTime = millis();
                return true;
            }
        }
    }
    return false;
}

bool WeatherDisplay::checkEditZipButtonPressed() {
    static unsigned long lastEditTime = 0;
    if (M5.Touch.ispressed()) {
        auto pos = M5.Touch.getPressPoint();
        int x = pos.x;
        int y = pos.y;
        // Right-bottom button area remains unchanged
        if (x >= M5.Lcd.width() - 150 && x <= M5.Lcd.width() - 10 && y >= M5.Lcd.height() - 60 && y <= M5.Lcd.height() - 20) {
            if (millis() - lastEditTime > 300) {
                lastEditTime = millis();
                return true;
            }
        }
    }
    return false;
}

void WeatherDisplay::drawWeatherImage(String iconId, int resizeMult) {
    // Retrieve the bitmap data for the given icon
    const uint16_t * weatherBitmap = getWeatherBitmap(iconId);
    if (weatherBitmap == nullptr) return;
    
    int sWidth = M5.Lcd.width();
    int sHeight = M5.Lcd.height();
    int yOffset = -(resizeMult * imgSqDim - sHeight) / 2 - 30;
    int xOffset = sWidth - (imgSqDim * resizeMult * 0.8); // right-aligned
    
    // Iterating through the icons bitmap data and drawing pixels
    for (int y = 0; y < imgSqDim; y++) {
        for (int x = 0; x < imgSqDim; x++) {
            int pixNum = (y * imgSqDim) + x;
            uint16_t pixel = pgm_read_word(&(weatherBitmap[pixNum]));
            if (pixel != 0) {
                for (int i = 0; i < resizeMult; i++) {
                    for (int j = 0; j < resizeMult; j++) {
                        int xDraw = x * resizeMult + i + xOffset;
                        int yDraw = y * resizeMult + j + yOffset;
                        M5.Lcd.drawPixel(xDraw, yDraw, pixel);
                    }
                }
            }
        }
    }
}
