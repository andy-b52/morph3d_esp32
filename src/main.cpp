#include <Arduino.h>
#include "DrawUtils.h"
#include "Models.h"
#include "WebModule.h"
#include "WiFiModule.h"
#include "esp_system.h"

const int TARGET_FPS = 60;  // frames per second
const int FRAME_INTERVAL_MS = 1000 / TARGET_FPS;

unsigned long lastFrameSentTime = 0;
uint32_t frameNumber = 0;
std::vector<uint16_t> encodedFrame;
unsigned long lastSerialPrintTime = 0;

void drawRandomPixel() {
    uint8_t c = static_cast<uint8_t>(rand() % 256);

    int x = rand() % WIDTH;
    int y = rand() % HEIGHT;

    setPixel(x, y, c);
}

void printInfo() {
    Serial.println("Frame #" + String(frameNumber) + " sent");

    Serial.println("encodedFrame capacity: " + String(encodedFrame.capacity()));

    // Internal RAM available for dynamic allocation
    Serial.print("FH: " + String(ESP.getFreeHeap()) + " bytes, ");

    // Largest contiguous block available in the heap
    Serial.print("LFHB: " + String(ESP.getMaxAllocHeap()) + " bytes, ");

    // Total heap currently available
    Serial.print("TH: " + String(ESP.getHeapSize()) + " bytes, ");

    // Minimum free heap observed since startup
    Serial.print("MFH: " + String(ESP.getMinFreeHeap()) + " bytes\n");
}

const char* getResetReason(esp_reset_reason_t reason) {
    switch (reason) {
        case ESP_RST_UNKNOWN:
            return "Unknown reset reason";
        case ESP_RST_POWERON:
            return "Power-on reset";
        case ESP_RST_EXT:
            return "External reset";
        case ESP_RST_SW:
            return "Software reset";
        case ESP_RST_PANIC:
            return "Exception / panic";
        case ESP_RST_INT_WDT:
            return "Interrupt watchdog";
        case ESP_RST_TASK_WDT:
            return "Task watchdog";
        case ESP_RST_WDT:
            return "Other watchdog";
        case ESP_RST_DEEPSLEEP:
            return "Wake from deep sleep";
        case ESP_RST_BROWNOUT:
            return "Brownout (low supply voltage)";
        case ESP_RST_SDIO:
            return "SDIO reset";
        default:
            return "Other / unrecognized reset";
    }
}

void setup() {
    Serial.begin(115200);

    delay(500);

    esp_reset_reason_t reason = esp_reset_reason();

    Serial.println("Reset reason: " + String(getResetReason(reason)));

    WiFiModule::begin();

    WebModule::begin();

    createModels();
}

void loop() {
    WebModule::wsCleanupClients();

    if (WebModule::wsNumberOfConnectedClients() == 0) {
        return;  // no connected client...
    }

    static bool nextFrameDrawn = false;

    if (!nextFrameDrawn) {
        // drawRandomPixel();
        drawFrame(frameNumber);

        nextFrameDrawn = true;
    }

    unsigned long now = millis();

    if (now - lastFrameSentTime < FRAME_INTERVAL_MS) {
        delay(1);
        return;
    }

    if (!WebModule::wsAvailableForWrite()) {
        // Serial.println("WebSocket is not available for write!");
        return;
    }

    // int LFBS = (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    // if (LFBS < FRAME_BYTES){
    //     Serial.println("No memory to send the frame...");
    //     return;
    // }

    lastFrameSentTime = now;

    encodeFrame(encodedFrame);

    nextFrameDrawn = false;

    // WebModule::wsSendBuf(frameBuf, FRAME_BYTES);
    WebModule::wsSendBuf(encodedFrame.data(),
                         encodedFrame.size() * sizeof(uint16_t));

    if (now - lastSerialPrintTime > 1000) {
        lastSerialPrintTime = now;
        printInfo();
    }

    frameNumber++;
}
