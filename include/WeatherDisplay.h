#ifndef WEATHERDISPLAY_H
#define WEATHERDISPLAY_H

#include <ArduinoJson.h>
#include <M5Core2.h>
#include <HTTPClient.h>

class WeatherDisplay {
public:
    String cityName;
    String strWeatherDesc;
    String strWeatherIcon;
    double tempNow;
    double tempMin;
    double tempMax;
    String lastSyncTime;
    
    WeatherDisplay();
    void init(String apiKey);
    void updateWeather(String zip);
    void draw(bool isFahrenheit);
    bool checkToggleButtonPressed();
    bool checkEditZipButtonPressed();
    
private:
    String apiKey;
    String urlOpenWeather = "https://api.openweathermap.org/data/2.5/weather?";
    // Helper for making HTTP GET requests
    String httpGETRequest(const char* serverURL);
    // Draws the weather icon using bitmap data from the provided header file
    void drawWeatherImage(String iconId, int resizeMult);
};

#endif
