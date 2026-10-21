#include <Arduino.h>
#include "FrameBuffer.h"
#include "Models.h"
#include "Morpher.h"
#include "Renderer.h"
#include "RleEncoder.h"
#include "WebModule.h"
#include "WiFiModule.h"
#include "esp_system.h"

namespace {
const int TARGET_FPS = 60;  // frames per second
const int FRAME_INTERVAL_MS = 1000 / TARGET_FPS;

const size_t ENCODED_FRAME_RESERVE = 12 * 1024;

unsigned long lastSerialPrintTime = 0;

unsigned long lastFrameSentTime = 0;
uint32_t frameNumber = 0;
std::vector<uint8_t> encodedFrame;

Morpher morpher;

void drawFrame(uint32_t frameNr) {
    fadeOutFrame();
    renderModel(morpher.step(), frameNr);
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
}  // namespace

void setup() {
    Serial.begin(115200);

    delay(500);

    esp_reset_reason_t reason = esp_reset_reason();

    Serial.println("Reset reason: " + String(getResetReason(reason)));

    WiFiModule::begin();

    WebModule::begin();

    getModels();

    encodedFrame.reserve(ENCODED_FRAME_RESERVE);
}

void loop() {
    WebModule::wsCleanupClients();

    if (WebModule::wsNumberOfConnectedClients() == 0) {
        delay(50);  // no clients: nothing to render, check again soon
        return;
    }

    static bool nextFrameDrawn = false;

    if (!nextFrameDrawn) {
        drawFrame(frameNumber);

        nextFrameDrawn = true;
    }

    unsigned long now = millis();

    if (now - lastFrameSentTime < FRAME_INTERVAL_MS) {
        delay(1);
        return;
    }

    if (!WebModule::wsAvailableForWrite()) {
        delay(1);  // a client's send queue is still draining
        return;
    }

    // int LFBS = (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    // if (LFBS < FRAME_BYTES) {
    //     Serial.println("No memory to send the frame...");
    //     return;
    // }

    lastFrameSentTime = now;

    encodeFrame(frameBuf, FRAME_BYTES, encodedFrame);

    nextFrameDrawn = false;

    WebModule::wsSendBuf(encodedFrame.data(), encodedFrame.size());

    if (now - lastSerialPrintTime > 1000) {
        lastSerialPrintTime = now;
        printInfo();
    }

    frameNumber++;
}
