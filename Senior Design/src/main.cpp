#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <ESPmDNS.h>

#include "server_handler.h"

void handleButtonPressed();

const char* ssid = "Jasiek_2.4G";
const char* password = "Jasiek_2005";
int ledPin = 8;
int bttnPin = 9;

bool printToConsole = false;
int outsideTemp = 0;
int outsideHumidity = 0;
int batteryVoltage = 0;
int chargeCurrent = 0;
int solarPanelAngle = 0;
String activeStake = "";

enum event {
  NONE,
  BUTTON_PRESSED
};

volatile event systemEvent = NONE;

AsyncWebServer server(80);
AsyncEventSource events("/events");

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(bttnPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(bttnPin), handleButtonPressed, CHANGE);

  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  if (!MDNS.begin("garden")) {
    Serial.println("Error starting mDNS");
    return;
  }

  MDNS.addService("_http", "_tcp", 80);
  Serial.println("http://garden.local");

  registerServerCallbacks(server, events);
  server.begin();
}

void loop() {
  switch (systemEvent) {
    case BUTTON_PRESSED:
      if (printToConsole) {
        Serial.println("Button pressed!");
        events.send("Test", "button-pressed", millis());
      }
      systemEvent = NONE;
      break;
    default:
      delay(100);
      break;
  }
}

void handleButtonPressed() {
  if (digitalRead(bttnPin) == LOW) {
    systemEvent = BUTTON_PRESSED;
  }
}