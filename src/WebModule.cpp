#include "WebModule.h"
#include <ESPAsyncWebServer.h>

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
    const data32 = image.data;

    // Automatically match the ESP's current IP address
    const wsUri = `ws://${window.location.hostname}/ws`;
    let websocket;

    let receivedFrames = 0;
    let lastStatsTime = performance.now();    

    function initWebSocket() {
        console.log('Trying to open a WebSocket connection...');
        
        websocket = new WebSocket(wsUri);
        websocket.binaryType = "arraybuffer";
        
        websocket.onopen = function(event) {
            console.log('Connection opened');
            document.getElementById('ws-status').innerHTML = "Connected";
            document.getElementById('ws-status').style.color = "green";
        };
        
        websocket.onclose = function(event) {
            console.log('Connection closed');
            document.getElementById('ws-status').innerHTML = "Disconnected";
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

            // if (event.data.byteLength !== FRAME_BYTES) {
            //     console.warn(
            //         `Invalid frame size: ${event.data.byteLength}; ` +
            //         `expected ${FRAME_BYTES} bytes`
            //     );
            //     return;
            // }

            // Each pixel is four consecutive bytes: R, G, B, A.
            // image.data.set(new Uint8ClampedArray(event.data));

            // Blazing fast unrolled loop for Uint8ClampedArray
            // for (let i = 0; i < data32.length; i += 4) {
            //     data32[i]   = 255; // Red
            //     data32[i+1] = 0;   // Green (Already 0 by default from createImageData)
            //     data32[i+2] = 0;   // Blue  (Already 0 by default from createImageData)
            //     data32[i+3] = 255; // Alpha
            // }            

            const colorSource = new Uint8Array(event.data);

            let len = 0;
            for (let i = 0; i < colorSource.length; i += 2) {
                len += colorSource[i + 1]; // the length byte
            }
            if (len !== FRAME_BYTES) {
                console.warn(
                    `Invalid frame size: ${len}; ` +
                    `expected ${FRAME_BYTES} bytes`
                );
                return;
            }

            let colorIndex = 0;
            let rgbaIndex = 0;

            // Loop through the pixels and inject the 4th (Alpha) channel
            while (colorIndex < colorSource.length) {
                let n = colorSource[colorIndex + 1];
                for (let i = 0; i < n; i++) {
                    data32[rgbaIndex + 0] = colorSource[colorIndex]; // R
                    data32[rgbaIndex + 1] = colorSource[colorIndex]; // G
                    data32[rgbaIndex + 2] = colorSource[colorIndex]; // B
                    data32[rgbaIndex + 3] = 255;                     // A (Always fully opaque)

                    rgbaIndex += 4; // Move 4 bytes forward in the target 32-bit canvas array
                }
                colorIndex += 2;  // Move 2 bytes forward in the source 8-bit array                    
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

void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len) {
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
    // return ws.availableForWriteAll();

    for (AsyncWebSocketClient& client : ws.getClients()) {
        if (client.status() == WS_CONNECTED && client.queueLen() != 0) {
            return false;
        }
    }

    return true;
}

void WebModule::wsSendBuf(const void* pBuf, size_t len) {
    for (AsyncWebSocketClient& client : ws.getClients()) {
        if (client.status() == WS_CONNECTED && client.canSend()) {
            client.binary(static_cast<const uint8_t*>(pBuf), len);
        }
    }
}