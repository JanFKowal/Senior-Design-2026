#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "WiFiProv.h"
#include "SPIFFS.h"
#include <ESPmDNS.h>

#include "server_handler.h"

void handleButtonPressed();
void SysProvEvent(arduino_event_t *event);

//ssid = "Jasiek_2.4G"
//password = "Jasiek_2005"

// const char* ssid = "";
// const char* password = "";
const char* pop = "gardenapp";// proof of possession for WiFi provisioning
const char* serviceName = "PROV_GardenESP"; // WiFi provisioning service name

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

  Serial.println("Before provisioning");
  WiFi.onEvent(SysProvEvent);
  Serial.println("After provisioning");

  if (digitalRead(bttnPin) == LOW) {
    Serial.println("Button held during startup. Clearing saved Wi-Fi credentials.");
    WiFi.disconnect(true, true);
    delay(500);
  }

  Serial.println("Before Wifi");
  WiFi.begin();
  Serial.println("After Wifi");

  if (WiFi.SSID().isEmpty()) {
    Serial.println("No saved Wi-Fi credentials. Begin provisioning using BLE");

    // uint8_t uuid[16] = {
    //   0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
    //   0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02
    // };
    WiFiProv.beginProvision(
      WIFI_PROV_SCHEME_BLE,
      WIFI_PROV_SCHEME_HANDLER_FREE_BLE,
      WIFI_PROV_SECURITY_1,
      pop,
      serviceName,
      NULL,
      NULL,//uuid,
      false
    );
  } else {
    Serial.print("Saved Wi-Fi credentials found for ");
    Serial.println(WiFi.SSID());
  }
  Serial.println("Before wifi loop");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  if (!MDNS.begin("garden")) {
    Serial.println("Error starting mDNS");
    return;
  }

  MDNS.addService("_http", "_tcp", 80);

  registerServerCallbacks(server, events);
  Serial.print("Starting web server at http://");
  Serial.println(WiFi.localIP());
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

void SysProvEvent(arduino_event_t *sys_event) {
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("\nConnected IP address : ");
      Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: Serial.println("\nDisconnected. Connecting to the AP again... "); break;
    case ARDUINO_EVENT_PROV_START:            Serial.println("\nProvisioning started\nGive Credentials of your access point using smartphone app"); break;
    case ARDUINO_EVENT_PROV_CRED_RECV:
      Serial.println("\nReceived Wi-Fi credentials");
      Serial.print("\tSSID : ");
      Serial.println((const char *)sys_event->event_info.prov_cred_recv.ssid);
      Serial.print("\tPassword : ");
      Serial.println((char const *)sys_event->event_info.prov_cred_recv.password);
      break;
    case ARDUINO_EVENT_PROV_CRED_FAIL:
      Serial.println("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n");
      if (sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR) {
        Serial.println("\nWi-Fi AP password incorrect");
      } else {
        Serial.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
      }
      break;
    case ARDUINO_EVENT_PROV_CRED_SUCCESS: Serial.println("\nProvisioning Successful"); break;
    case ARDUINO_EVENT_PROV_END:          Serial.println("\nProvisioning Ends"); break;
    default:                              break;
  }
}
