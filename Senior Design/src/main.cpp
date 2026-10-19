// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

//Initalize method names
void handleTestModeOn(AsyncWebServerRequest *request);
void handleTestModeOff(AsyncWebServerRequest *request);
void handleWaterAll(AsyncWebServerRequest *request);
void handleStopWaterAll(AsyncWebServerRequest *request);
void registerServerCallbacks();
void handleState(AsyncWebServerRequest *request);

// Replace with your network credentials
const char* ssid = "Jasiek_2.4G";
const char* password = "Jasiek_2005";
const int ledPin = 8; // GPIO pin for the LED

bool printToConsole = false;
int outsideTemp = 0;
int outsideHumidity = 0;
int batteryVoltage = 0;
int chargeCurrent = 0;
int solarPanelAngle = 0;
String activeStake = "";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  registerServerCallbacks();

  // Start server
  server.begin();
}
 
void loop(){
  delay(1000);
}

void registerServerCallbacks() {
    // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Register button press handlers
  server.on("/test/on", HTTP_GET, handleTestModeOn);
  server.on("/test/off", HTTP_GET, handleTestModeOff);
  server.on("/water/all", HTTP_GET, handleWaterAll);
  server.on("/stopwater/all", HTTP_GET, handleStopWaterAll);
  
  server.on("/api/state", HTTP_GET, handleState);
}
/** Handlers**************************** */

/*
  * Handles the request to enable test mode
*/
  void handleTestModeOn(AsyncWebServerRequest *request) {
    printToConsole = true;
    //blink LED 3 times
    for(int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      delay(200);
      digitalWrite(ledPin, LOW);
      delay(200);
    }
    Serial.println("Test mode enabled");
    request->send(200, "text/plain", "Test mode enabled");
  }
  /*
  * Handles the request to disable test mode
  */
  void handleTestModeOff(AsyncWebServerRequest *request) {
    printToConsole = false; 
    request->send(200, "text/plain", "Test mode disabled");
  }
  /*
  * Handles the request to water all plants
  */
  void handleWaterAll(AsyncWebServerRequest *request) {
    if (printToConsole) {
      Serial.println("Adding all plants to watering queue...");
    }
    // TODO: Add logic for adding to queue
    activeStake = "Stake 1"; // Set active stake
    request->send(200, "text/plain", "Watering all plants");
  }
  /*
  * Handles the request to stop all watering cycles
  */
  void handleStopWaterAll(AsyncWebServerRequest *request) {
    if (printToConsole) {
      Serial.println("Stopping all watering cycles...");
    }
    // TODO: Add logic for stopping all watering cycles
    activeStake = ""; // Clear active stake
    request->send(200, "text/plain", "All watering cycles stopped");
  }

  /*
  * Handles the API request for current system values
  */
  void handleState(AsyncWebServerRequest *request) {
    
    String jsonResponse = "{";
    jsonResponse += "\"printToConsole\": " + String(printToConsole ? "true" : "false") + ",";
    jsonResponse += "\"outsideTemp\": " + String(outsideTemp) + ",";
    jsonResponse += "\"outsideHumidity\": " + String(outsideHumidity) + ",";
    jsonResponse += "\"batteryVoltage\": " + String(batteryVoltage) + ",";
    jsonResponse += "\"chargeCurrent\": " + String(chargeCurrent) + ",";
    jsonResponse += "\"activeStake\": \"" + activeStake + "\",";
    jsonResponse += "\"solarPanelAngle\": " + String(solarPanelAngle);
    jsonResponse += "}";

    request->send(200, "application/json", jsonResponse);
  }