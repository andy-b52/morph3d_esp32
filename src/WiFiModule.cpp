#include "WiFiModule.h"
#include <WiFi.h>
#include "secrets.h"

#ifdef USE_SOFT_AP
static_assert(sizeof(SOFT_AP_PASS) - 1 >= 8,
              "SOFT_AP_PASS must be at least 8 characters (WPA2)");
static_assert(sizeof(SOFT_AP_PASS) - 1 <= 63,
              "SOFT_AP_PASS must be at most 63 characters (WPA2)");
static_assert(sizeof(SOFT_AP_SSID) - 1 <= 32,
              "SOFT_AP_SSID must be at most 32 characters");
#endif

namespace {
void OnWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("ESP32 connected to WiFi network");
            break;
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

#if defined(USE_SOFT_AP) && defined(USE_WIFI_NETWORK)
    // setting WiFi to both station and access point mode
    WiFi.mode(WIFI_MODE_APSTA);
#elif defined(USE_SOFT_AP)
    // setting WiFi to access point mode only
    WiFi.mode(WIFI_MODE_AP);
#elif defined(USE_WIFI_NETWORK)
    WiFi.mode(WIFI_MODE_STA);
#else
#error "None WiFi mode is defined"
#endif

#ifdef USE_SOFT_AP
    // opening an access point
    WiFi.softAP(SOFT_AP_SSID, SOFT_AP_PASS);

    Serial.print("Access Point IP address: ");
    Serial.println(WiFi.softAPIP());
#endif

#ifdef USE_WIFI_NETWORK
    // connecting to an existing WiFi network
    WiFi.begin(WIFI_NETWORK_SSID, WIFI_NETWORK_PASS);
    WiFi.setSleep(false);  // modem sleep delays incoming TCP ACKs and stalls streaming

    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connecting to WiFI...");
        delay(500);
    }

    Serial.print("IP address on WiFi network: ");
    Serial.println(WiFi.localIP());
#endif
}