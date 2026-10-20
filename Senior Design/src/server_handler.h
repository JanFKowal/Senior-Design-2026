#pragma once // same this as #ifndef ... #define ... #endif but simpler

#include "ESPAsyncWebServer.h"

void registerServerCallbacks(AsyncWebServer& server, AsyncEventSource& events);
