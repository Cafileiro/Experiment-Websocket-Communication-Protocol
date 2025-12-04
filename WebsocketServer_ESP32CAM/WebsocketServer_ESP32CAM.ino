#include <WiFi.h>
#include <esp32cam.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* WIFI_SSID = "LMAO";
const char* WIFI_PASS = "LOL";

AsyncWebSocket server(80); //servidor en el puerto 80
AsyncWebServer ws("/ws");

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


void checkCamera()  //checkCamera
{
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));
}

void handleJpgLo()  //permite enviar la resolucion de imagen baja
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}

void handleJpgHi()  //permite enviar la resolucion de imagen alta
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
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

  if(confAndCheckCamera() == false){Serial.println("Camera fail")}else{Serial.println("Camera OK")}

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);  //nos conectamos a la red wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/cam-lo.jpg");  //para conectarnos IP res baja

  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/cam-hi.jpg");  //para conectarnos IP res alta

  server.on("/cam-lo.jpg", handleJpgLo);  //enviamos al servidor
  server.on("/cam-hi.jpg", handleJpgHi);

  server.begin();
}

void loop() {
  server.handleClient();
}
