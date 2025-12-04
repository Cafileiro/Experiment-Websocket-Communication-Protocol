#include <WiFi.h>
#include <esp32cam.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <esp_camera.h>

const char* WIFI_SSID = "LMAO"; 
const char* WIFI_PASS = "LOL";

AsyncWebSocket ws("/ws"); 
AsyncWebServer server(80);//servidor en el puerto 80

static auto loRes = esp32cam::Resolution::find(320, 240);  //baja resolucion
static auto hiRes = esp32cam::Resolution::find(800, 600);  //alta resolucion

const unsigned long CAPTURE_INTERVAL = 1000;  

unsigned long lastCapture = 0;

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client,
               AwsEventType type, void * arg, uint8_t * data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("WS client %u connected\n", client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("WS client %u disconnected\n", client->id());
  } else if (type == WS_EVT_DATA) {
    // If you want to receive commands from the Node server you can parse them here.
    Serial.printf("Received data from client %u, len=%u\n", client->id(), (unsigned)len);
  }
}

bool confAndCheckCamera(){
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    return Camera.begin(cfg);
  }

}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();

  if(confAndCheckCamera() == false){Serial.println("Camera fail");}else{Serial.println("Camera OK");}
  // Wifi
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);  //nos conectamos a la red wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }


  Serial.println(WiFi.localIP());

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);


  server.begin();
  Serial.println("Server started");
}

void loop() {
  unsigned long now = millis();
  if (now - lastCapture >= CAPTURE_INTERVAL) {
    lastCapture = now;
    if (ws.count() > 0) { // only capture if clients connected
      camera_fb_t * fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
      }

      // Send JPEG binary to all websocket clients
      // Note: AsyncWebSocket::binaryAll accepts uint8_t*, size_t
      ws.binaryAll(fb->buf, fb->len);

      esp_camera_fb_return(fb);
      Serial.printf("Sent frame, %u bytes, clients=%d\n", (unsigned)fb->len, ws.count());
    }
  }
  // nothing else needed in loop for Async server
}
