#include <Arduino.h>
#include <PrivateConfig.h>
#include <Logger.h>
#include <NetHandler.hpp>
#include <EspNowHandler.hpp>
#include <DataMessageQueue.hpp>
#include <HTTPClient.h>
#include <string.h>
#include <OTE.hpp>
#include <ArduinoOTA.h>
#include <GwSdCard.hpp>
#include <WebServer.hpp>
#include <ETH.h>
#include <WiFi.h>
#include <SSD1306.h>

#define OLIMEX_BUTTON1 34

SSD1306Wire display(0x3c, SCL, SDA ); // SCL and SDA defined in platformio.ini

struct tm timeinfo;
bool ntpInitialized;
bool otaInitialized;
bool espNowInitialized;
bool webServerInitialized;

volatile int watchdogCounter;
hw_timer_t *watchdogTimer = NULL;

unsigned long displayTimestamp = 0l;
unsigned int displayPage = 0;
unsigned long lastMillis = 0l;
unsigned long loopTime = 0l;


void IRAM_ATTR watchdogFunction()
{
  if ( watchdogCounter++ >= 120 )
  {
    timerEnd( watchdogTimer );
    Serial.println();
    LOGM( "*** RESET by watchdog ***" );
    delay( 3000 );
    ESP.restart();
    delay( 5000 );
  }
}

void sendValue(String url)
{
  HTTPClient http;

  // LOG();
  // Serial.println( url );
  LOGF( "url=%s", url.c_str());

  http.begin(url);
  http.setAuthorization( OHAB_USER, OHAB_PASS );

  LOGM( "Connecting OpenHAB..." );
  int httpCode = http.GET();

  if(httpCode > 0)
  {
    // LOG();
    // Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    LOGF("[HTTP] GET... code: %d", httpCode);

    if(httpCode == HTTP_CODE_OK)
    {
      LOGM( "reading payload" );
      // delay(10);
      // String payload = http.getString();
      // LOG();
      // Serial.println(payload);
      // LOGF( "payload=%s", payload.c_str());
    }
  }
  else
  {
    // LOG();
    // Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    LOGF("[HTTP] GET... failed, error: %s", http.errorToString(httpCode).c_str());
  }

  http.end();
  LOGM("Connection closed.");
}


void setup()
{
  Serial.begin(115200);
  pinMode( OLIMEX_BUTTON1, INPUT );
  delay(100);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "EspOhab-Gateway");
  display.drawString(0, 12, "Version: " APP_VERSION );
  display.drawString(0, 24, "Build Date: " __DATE__ );
  display.drawString(0, 36, "Build Time: " __TIME__ );
  display.drawString(0, 48, "Booting...");

  display.display();

  delay( 3000 ); // wait for serial monitor
  Serial.println( "\n\n\nESP32 EspNow OpenHab Gateway - Version " APP_VERSION " by Dr. Thorsten Ludewig" );
  Serial.printf("ESP32 Arduino SDK Version: %s\n", ESP.getSdkVersion() );
  LOGM( "Build date: " __DATE__ " " __TIME__ );
  //
  watchdogCounter = 0;
  watchdogTimer = timerBegin( 0, 80, true );
  timerAttachInterrupt( watchdogTimer, &watchdogFunction, true );
  timerAlarmWrite( watchdogTimer, 1000000, true );
  timerAlarmEnable( watchdogTimer );
  LOGM( "Watchdog enabled" );
  //
  ntpInitialized = false;
  otaInitialized = false;
  espNowInitialized = false;
  webServerInitialized = false;
  //
  displayTimestamp = 0;
  displayPage = 0;
  //
  WiFi.mode( WIFI_OFF );
  WiFi.onEvent(NetEventHandler);
  InitializeDataMessageQueue();
  InitializeEspNowHandler();
  espNowInitialized = true;
  InitializeSdCard();
  LOGM( "Start ETH ...");
  ETH.begin();
  LOGM( "ETH set hostname");
  ETH.setHostname(ETH_HOSTNAME);
  LOGM("Setup finnished");
}

void loop()
{
  if( eth_connected )
  {
    watchdogCounter = 0;

    loopTime = millis() - lastMillis;
    lastMillis = millis();

    if( otaInitialized )
    {
      ArduinoOTA.handle();
    }
    else
    {
      InitializeOTE();
      otaInitialized = true;
    }

    if( !webServerInitialized )
    {
      InitializeWebServer();
      webServerInitialized = true;
    }

    if ( !ntpInitialized )
    {
      LOGM( "Config SNTP");

      configTzTime(TZ_INFO, NTP_SERVER1);

      LOGM( "Receive local time...");

      if (getLocalTime(&timeinfo, 10000))    // wait up to 10sec to sync
      {
        LOG();
        Serial.println(&timeinfo, "Time set: %A %d %B %Y %H:%M:%S");
        LOG();
        Serial.print( "Timezone: ");
        Serial.println( getenv("TZ"));
        ntpInitialized = true;
      }
      else
      {
        LOGM("Time not set");
      }
    }

    if ( digitalRead( OLIMEX_BUTTON1 ) == LOW )
    {
      LOGM( "Test button pressed." );
      LOG();
      Serial.printf( "Loop time = %ld\n", loopTime );
      sendValue( OHAB_HOST OHAB_TEST );

      getLocalTime(&timeinfo);

      LOG();
      Serial.println(&timeinfo, "Time: %Y-%m-%d %H:%M:%S");

      time_t now;
      time( &now );
      LOG();
      Serial.print( "now=");
      Serial.println( now );

      unsigned long uptime = millis();
      int upmillis = uptime % 1000; uptime /= 1000;
      int upsecs = uptime % 60; uptime /= 60;
      int upmins = uptime % 60; uptime /= 60;
      int uphours = uptime % 24; uptime /= 24;
      int updays = uptime;

      LOG();
      Serial.printf( "uptime: %dd %dh %dm %ds %dms\n", updays, uphours, upmins, upsecs, upmillis );

      TestSdCard();

      delay( 500 );
    }

    if ( millis() - displayTimestamp >= 5000 )
    {
      // Serial.println( displayPage );
      display.clear();

      switch( displayPage )
      {
        case 0:
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.setFont(ArialMT_Plain_10);
          display.drawString(0, 0, "ESP OpenHab Gateway");
          display.drawString(0, 12, "Dr. Thorsten Ludewig");
          display.drawString(0, 24, "Version: " APP_VERSION );
          display.drawString(0, 36, "Build Date: " __DATE__ );
          display.drawString(0, 48, "Build Time: " __TIME__ );
          break;

        case 1:
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.setFont(ArialMT_Plain_10);
          display.drawString(0, 0, "ETHERNET");
          display.drawString(5, 14, "Hostname:");
          display.drawString(10, 24, ETH.getHostname());
          display.drawString(5, 38, "IPv4 Address:");
          display.drawString(10, 48, ETH.localIP().toString());
          break;

        case 2:
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.setFont(ArialMT_Plain_10);
          display.drawString(0, 0, "WIFI");
          display.drawString(5, 14, "BSSID:");
          display.drawString(10, 24, WIFI_SSID );
          if ( espNowInitialized )
          {
            display.drawString(5, 38, "MAC Address:");
            display.drawString(10, 48, WiFi.softAPmacAddress());
          }
          break;

        case 3:
        {
          char buffer[16];
          getLocalTime(&timeinfo);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.setFont(ArialMT_Plain_10);
          display.drawString(0, 0, "TIME");
          display.setFont(ArialMT_Plain_16);
          sprintf( buffer, "%d-%02d-%02d", 1900 + timeinfo.tm_year, 1 + timeinfo.tm_mon, timeinfo.tm_mday );
          display.drawString(10, 20, buffer );
          sprintf( buffer, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec );
          display.drawString(10, 40, buffer );
        }
      }

      display.display();
      displayPage++;
      displayPage = displayPage % 4;
      displayTimestamp = millis();
    }

    if( dataMqAvailable() )
    {
      GwData* data = dataMqReceive();

      LOG();
      Serial.printf( "data.name = %s\n", data->name );

      LOG();
      Serial.print( "data.counter = " );
      Serial.println( data->counter );

      LOG();
      Serial.print( "data.timestamp = " );
      Serial.println( data->timestamp );

      LOG();
      Serial.print( "data.battery = " );
      Serial.println( data->battery );

      LOG();
      Serial.printf( "data.action = %s\n", data->action );

      String url = OHAB_HOST + String(data->action);

      sendValue( url );

      if( data->battery > 0 )
      {
        char buffer[128];
        sprintf( buffer, "%sCMD?%s=%.2f", OHAB_HOST, data->name, data->battery );
        sendValue( String(buffer));
      }
    }
  }
}
