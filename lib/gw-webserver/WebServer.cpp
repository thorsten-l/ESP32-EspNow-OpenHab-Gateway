#include "WebServer.hpp"
#include <Logger.h>
#include <PrivateConfig.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SD_MMC.h>

#include <WiFi.h>
#include <ETH.h>

#include <AsyncJson.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);

void jsonResponse( AsyncWebServerRequest *request, JsonObject &root )
{
  AsyncResponseStream *response = request->beginResponseStream("text/json");

  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Access-Control-Allow-Methods", "GET");

  root.printTo(*response);
  request->send(response);
}

void apiTime( AsyncWebServerRequest *request )
{
  char uptimeText[64];
  char timeText[64];

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  unsigned long uptime = millis();
  root["uptime"] = uptime;

  int upmillis = uptime % 1000; uptime /= 1000;
  int upsecs = uptime % 60; uptime /= 60;
  int upmins = uptime % 60; uptime /= 60;
  int uphours = uptime % 24; uptime /= 24;
  int updays = uptime;

  sprintf( uptimeText, "%dd %dh %dm %ds %dms", updays, uphours, upmins, upsecs, upmillis );

  root["uptimeText"] = uptimeText;

  time_t now;
  time( &now );
  root["time"] = now;
  root["timezone"] = getenv("TZ");

  struct tm timeinfo;
  getLocalTime(&timeinfo);

  sprintf( timeText, "%d-%02d-%02d %02d:%02d:%02d", 1900 + timeinfo.tm_year, 1 + timeinfo.tm_mon,
        timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec );
  root["timeText"] = timeText;

  jsonResponse( request, root );
}

void apiNetwork( AsyncWebServerRequest *request )
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  root["wifiSsid"] = WIFI_SSID;
  root["wifiHideSsid"] = WIFI_HIDE_SSID;
  root["wifiChannel"] = WIFI_CHNL;
  root["wifiMacAddress"] = WiFi.softAPmacAddress();

  root["otaHostname"] = OTA_HOSTNAME ".local";

  root["ethHostname"] = ETH_HOSTNAME;
  root["ethMacAddress"] = ETH.macAddress();
  root["ethIPv4Address"] = ETH.localIP().toString();
  root["ethIPv6Address"] = ETH.localIPv6().toString();

  root["openHabUrl"] = OHAB_HOST;

  jsonResponse( request, root );
}

void apiSystem( AsyncWebServerRequest *request )
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  root["appName"] = "ESP32 EspNow OpenHAB Gateway";
  root["appVersion"] = APP_VERSION;
  root["appAuthor"] = "Dr. Thorsten Ludewig <t.ludewig@gmail.com>";
  root["sdkVersion"] = ESP.getSdkVersion();
  root["buildDate"] = __DATE__;
  root["buildTime"] = __TIME__;

  jsonResponse( request, root );
}

void InitializeWebServer()
{
  server.on( "/api/time", HTTP_GET, apiTime );
  server.on( "/api/network", HTTP_GET, apiNetwork );
  server.on( "/api/system", HTTP_GET, apiSystem );

  server.serveStatic("/", SD_MMC, "/").setDefaultFile("index.html");

  server.onNotFound([](AsyncWebServerRequest *request)
  {
    Serial.printf("NOT_FOUND: ");

    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else
    if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else
    if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else
    if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else
    if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else
    if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else
    if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");

    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    request->send(404);
  });

  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    LOGM("onRequestBody");

    if(!index)
      Serial.printf("BodyStart: %u\n", total);

    Serial.printf("%s", (const char*)data);

    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });

  server.begin();
  LOGM( "WebServer initialized");
}
