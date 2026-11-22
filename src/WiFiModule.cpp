#include "WiFiModule.h"
#include <WiFi.h>
#include "secrets.h"

static_assert(sizeof(SOFT_AP_PASS) - 1 >= 8,
              "SOFT_AP_PASS must be at least 8 characters (WPA2)");
static_assert(sizeof(SOFT_AP_PASS) - 1 <= 63,
              "SOFT_AP_PASS must be at most 63 characters (WPA2)");
static_assert(sizeof(SOFT_AP_SSID) - 1 <= 32,
              "SOFT_AP_SSID must be at most 32 characters");

namespace {
void OnWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_AP_START:
            Serial.println("ESP32 soft AP started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            Serial.println("Station connected to ESP32 soft AP");
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            Serial.println("Station disconnected from ESP32 soft AP");
            break;
        default:
            break;
    }
}
}

void WiFiModule::begin() {
    WiFi.onEvent(OnWiFiEvent);
    WiFi.mode(WIFI_MODE_AP);

    // opening an access point
    WiFi.softAP(SOFT_AP_SSID, SOFT_AP_PASS);

    Serial.print("Access Point IP address: ");
    Serial.println(WiFi.softAPIP());
}