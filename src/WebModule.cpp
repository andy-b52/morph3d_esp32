#include "WebModule.h"
#include <ESPAsyncWebServer.h>

static_assert(WS_MAX_QUEUED_MESSAGES == 1,
              "wsAvailableForWrite() relies on a one-message queue; "
              "set -D WS_MAX_QUEUED_MESSAGES=1 in platformio.ini build_flags");

namespace {
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Video Stream</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            background-color: #f4f4f9;
            margin-top: 50px;
        }
        canvas {
            width: 100%;
            max-width: 640px;
            height: auto;
            aspect-ratio: 4 / 3;
            image-rendering: pixelated;
            border: 1px solid #777;
            display: inline-block;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
        }
        .status {
            font-weight: bold;
            color: #555;
        }
        .stats {
            margin: 12px;
        }
    </style>
</head>
<body>
<div class="container">
    <h2>ESP32 Video Stream</h2>

    <p class="status">Connection: <span id="ws-status" style="color: red;">Disconnected</span></p>
    
    <canvas id="screen" width="320" height="240"></canvas>
    
    <p id="stats" class="stats">FPS: 0</p>
</div>

<script>
    const WIDTH = 320;
    const HEIGHT = 240;
    const BYTES_PER_PIXEL = 1;
    const FRAME_BYTES = WIDTH * HEIGHT * BYTES_PER_PIXEL;

    const canvas = document.getElementById("screen");
    const ctx = canvas.getContext("2d", { alpha: false });

    const image = ctx.createImageData(WIDTH, HEIGHT);
    const rgba = image.data;
    // Same memory as rgba, one element per pixel (R, G, B, A bytes packed)
    const pixels = new Uint32Array(rgba.buffer);

    // Grey level -> opaque packed pixel (little-endian: A B G R)
    const palette = new Uint32Array(256);
    for (let c = 0; c < 256; c++) {
        palette[c] = 0xFF000000 | (c << 16) | (c << 8) | c;
    }

    // Automatically match the ESP's current IP address
    const wsUri = `ws://${window.location.host}/ws`;
    let websocket;

    let receivedFrames = 0;
    let lastStatsTime = performance.now();    

    function initWebSocket() {
        console.log('Trying to open a WebSocket connection...');
        
        websocket = new WebSocket(wsUri);
        websocket.binaryType = "arraybuffer";
        
        websocket.onopen = function(event) {
            console.log('Connection opened');
            document.getElementById('ws-status').textContent = "Connected";
            document.getElementById('ws-status').style.color = "green";
        };
        
        websocket.onclose = function(event) {
            console.log('Connection closed');
            document.getElementById('ws-status').textContent = "Disconnected";
            document.getElementById('ws-status').style.color = "red";
            // Attempt to reconnect every 2 seconds
            setTimeout(initWebSocket, 2000); 
        };
        
        websocket.onmessage = function(event) {
            // console.log("Message received:", {
            //     type: typeof event.data,
            //     isArrayBuffer: event.data instanceof ArrayBuffer,
            //     size: event.data.byteLength
            // });

            if (!(event.data instanceof ArrayBuffer)) {
                console.warn("Message is not an ArrayBuffer");
                return;
            }

            if (event.data.byteLength % 2 !== 0) {
                console.warn(
                    `Invalid frame size: ${event.data.byteLength}; ` +
                    `expected even number of bytes`
                );
                return;
            }

            // Runs of [grey level, run length] byte pairs
            const runs = new Uint8Array(event.data);

            let pos = 0;
            for (let i = 0; i < runs.length; i += 2) {
                const n = runs[i + 1];
                if (pos + n > FRAME_BYTES) {
                    console.warn(`Invalid frame: more than ${FRAME_BYTES} pixels`);
                    return;
                }
                pixels.fill(palette[runs[i]], pos, pos + n);
                pos += n;
            }
            if (pos !== FRAME_BYTES) {
                console.warn(
                    `Invalid frame size: ${pos}; expected ${FRAME_BYTES} pixels`
                );
                return;
            }

            ctx.putImageData(image, 0, 0);

            receivedFrames++;

            const now = performance.now();
            const elapsed = now - lastStatsTime;

            if (elapsed >= 1000) {
                const fps = receivedFrames * 1000 / elapsed;

                document.getElementById("stats").textContent = `FPS: ${fps.toFixed(1)}`;

                receivedFrames = 0;
                lastStatsTime = now;
            }
        };
    }

    window.addEventListener('load', initWebSocket);
</script>

</body>
</html>
)html";

void onWsEvent(AsyncWebSocket*, AsyncWebSocketClient* client, AwsEventType type,
               void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.println("WebSocket client #" + String(client->id()) +
                       " connected from " + client->remoteIP().toString());
        // client->text("Hello from ESP32 Server!");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("WebSocket client #" + String(client->id()) +
                       " disconnected");
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len) {
            data[len] = 0;
            String msg = (char*)data;
            Serial.println("Received message: " + msg);
        }
    }
}
}  // namespace

void WebModule::begin() {
    ws.onEvent(onWsEvent);

    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", index_html);
    });

    server.begin();
}

void WebModule::wsCleanupClients() { ws.cleanupClients(); }

int WebModule::wsNumberOfConnectedClients() { return ws.count(); }

bool WebModule::wsAvailableForWrite() {
    // Locked inside the library. With a one-message queue, "not full"
    // means every client has finished sending its previous frame.
    return ws.availableForWriteAll();
}

void WebModule::wsSendBuf(const void* pBuf, size_t len) {
    // One copy shared by every client, freed after the last one has sent it
    ws.binaryAll(static_cast<const uint8_t*>(pBuf), len);
}
