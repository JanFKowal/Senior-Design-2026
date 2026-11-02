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
const char* serviceName = "PROV_GardenESP"; // WiFi provisioning service name that shows in app

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
  delay(500); // get serial monitor running
  pinMode(ledPin, OUTPUT);
  pinMode(bttnPin, INPUT_PULLUP);

  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.onEvent(SysProvEvent);
  WiFi.mode(WIFI_MODE_STA);// set WiFi to station mode so WiFi.disconnect() doesnt fail

  if (digitalRead(bttnPin) == LOW) {
    Serial.println("Button held during startup. Clearing saved Wi-Fi credentials.");
    bool success = WiFi.disconnect(true, true);
    if (!success) {
      Serial.println("Failed to clear Wi-Fi credentials.");
    }    
    delay(500);
  }
  attachInterrupt(digitalPinToInterrupt(bttnPin), handleButtonPressed, CHANGE);


  if (WiFi.SSID().isEmpty()) {
    Serial.println("No saved Wi-Fi credentials. Begin provisioning using BLE");

    WiFiProv.beginProvision(
      WIFI_PROV_SCHEME_BLE,
      WIFI_PROV_SCHEME_HANDLER_FREE_BLE,
      WIFI_PROV_SECURITY_1,
      pop,
      serviceName,
      NULL,
      NULL//uuid,
    );
  } else {
    Serial.print("Saved Wi-Fi credentials found for ");
    Serial.println(WiFi.SSID());

    WiFi.begin();
  }

  while (WiFi.status() != WL_CONNECTED) { //while not connected
    delay(1000);
    Serial.println("...");
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
        events.send("Test", "button-pressed");
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
 /*
  * Handle the system provisioning events.
  */
void SysProvEvent(arduino_event_t *sys_event) {
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_PROV_START: // Provisioning started
      Serial.println("\nProvisioning started.");
      Serial.println("Give your access point credentials using the smartphone app.");
      break;

    case ARDUINO_EVENT_PROV_CRED_RECV: // Received Wi-Fi credentials
      Serial.println("\nReceived Wi-Fi credentials.");
      Serial.print("\tSSID: ");
      Serial.println((const char *)sys_event->event_info.prov_cred_recv.ssid);
      break;

    case ARDUINO_EVENT_PROV_CRED_SUCCESS: // Provisioning successful
      Serial.println("\nProvisioning successful.");
      break;

    case ARDUINO_EVENT_PROV_END: // Provisioning ended
      Serial.println("\nProvisioning ended.");
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP: // Got IP from Wi-Fi
      Serial.print("\nConnected IP address: ");
      Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
      break;

    case ARDUINO_EVENT_PROV_CRED_FAIL: // Provisioning failed
      if (sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR) {
        Serial.println("\nProvisioning failed: Wi-Fi password is incorrect."); // Pass is wrong
      } else {
        Serial.println("\nProvisioning failed: Wi-Fi access point was not found."); // Wi-Fi not found
      }
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: // Disconnected from Wi-Fi
      Serial.println("\nDisconnected from Wi-Fi.");
      break;

    default:
      break;
  }
}
