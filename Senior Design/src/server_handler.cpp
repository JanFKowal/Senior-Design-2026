#include "server_handler.h"

#include "SPIFFS.h"

extern bool printToConsole;
extern int outsideTemp;
extern int outsideHumidity;
extern int batteryVoltage;
extern int chargeCurrent;
extern int solarPanelAngle;
extern String activeStake;
extern int ledPin;

namespace { // namespace equivalent to static definitions in C to privatize the functions to this file

void handleTestModeOn(AsyncWebServerRequest* request) {
  printToConsole = true;

  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    delay(100);
  }

  Serial.println("Test mode enabled");
  request->send(200, "text/plain", "Test mode enabled");
}

void handleTestModeOff(AsyncWebServerRequest* request) {
  printToConsole = false;
  request->send(200, "text/plain", "Test mode disabled");
}

void handleWaterAll(AsyncWebServerRequest* request) {
  if (printToConsole) {
    Serial.println("Adding all plants to watering queue...");
  }

  activeStake = "Stake 1";
  request->send(200, "text/plain", "Watering all plants");
}

void handleStopWaterAll(AsyncWebServerRequest* request) {
  if (printToConsole) {
    Serial.println("Stopping all watering cycles...");
  }

  activeStake = "";
  request->send(200, "text/plain", "All watering cycles stopped");
}

void handleGetStakeData(AsyncWebServerRequest* request) {
  if (!request->hasParam("name")) {
    request->send(400, "text/plain", "Missing stake name parameter");
    return;
  }

  String stakeName = request->getParam("name")->value();
  if (printToConsole) {
    Serial.println("Fetching data for " + stakeName);
  }

  const String response =
      "{\"waterLevel\":\"75%\","
      "\"timesArr\":[\"08:00\",\"16:30\"],"
      "\"vol\":\"Medium\","
      "\"daysArr\":[\"Mon\",\"Wed\",\"Fri\"]}";
  request->send(200, "application/json", response);
}

void handleSaveStakeData(AsyncWebServerRequest* request) {
  if (!request->hasParam("name") || !request->hasParam("times") ||
      !request->hasParam("vol") || !request->hasParam("days")) {
    request->send(400, "text/plain", "Missing parameters");
    return;
  }

  const String stakeName = request->getParam("name")->value();
  const String times = request->getParam("times")->value();
  const String vol = request->getParam("vol")->value();
  const String days = request->getParam("days")->value();

  if (printToConsole) {
    Serial.println("Saving data for " + stakeName);
    Serial.println("Times: " + times);
    Serial.println("Days: " + days);
    Serial.println("Volume: " + vol);
  }

  request->send(200, "text/plain", "Stake data saved");
}

void handleState(AsyncWebServerRequest* request) {
  String response = "{";
  response += "\"printToConsole\":" + String(printToConsole ? "true" : "false") + ",";
  response += "\"outsideTemp\":" + String(outsideTemp) + ",";
  response += "\"outsideHumidity\":" + String(outsideHumidity) + ",";
  response += "\"batteryVoltage\":" + String(batteryVoltage) + ",";
  response += "\"chargeCurrent\":" + String(chargeCurrent) + ",";
  response += "\"activeStake\":\"" + activeStake + "\",";
  response += "\"solarPanelAngle\":" + String(solarPanelAngle);
  response += "}";

  request->send(200, "application/json", response);
}

}  // namespace

void registerServerCallbacks(AsyncWebServer& server, AsyncEventSource& events) {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/test/on", HTTP_GET, handleTestModeOn);
  server.on("/test/off", HTTP_GET, handleTestModeOff);
  server.on("/water/all", HTTP_GET, handleWaterAll);
  server.on("/stopwater/all", HTTP_GET, handleStopWaterAll);
  server.on("/getstakedata", HTTP_GET, handleGetStakeData);
  server.on("/savestakedata", HTTP_GET, handleSaveStakeData);
  server.on("/api/state", HTTP_GET, handleState);
  server.addHandler(&events);
}
