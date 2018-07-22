#include "EspNowHandler.hpp"
#include "DataDefinition.h"
#include <esp_now.h>
#include <WiFi.h>
#include <Logger.h>
#include <PrivateConfig.h>
#include <DataMessageQueue.hpp>

static GwData data;

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *receivedData, int data_len ) {
  char macStr[18];

  LOGM( "data received" );

  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  LOG();
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  LOG();
  Serial.print("Last Packet Recv Data len : "); Serial.println(data_len);

  if( sizeof(data) == data_len )
  {
    LOGM( "Data size OK." );
    memcpy( &data, receivedData, data_len );
    dataMqSend(&data);
  }
  else
  {
    LOGM( "Wrong data size!" );
  }
}

void InitializeEspNowHandler()
{
  WiFi.mode(WIFI_AP);

  bool result = WiFi.softAP( WIFI_SSID, WIFI_PASS, WIFI_CHNL, WIFI_HIDE_SSID );
  delay(200);

  if (!result)
  {
    LOGM( "AP Config failed." );
  }
  else
  {
    LOGM( "AP Config Success. Broadcasting with AP: " WIFI_SSID );
    if ( WIFI_HIDE_SSID )
    {
      LOGM( "AP SSID: " WIFI_SSID " is HIDDEN.");
    }
    LOG();
    Serial.print( "AP MAC: " );
    Serial.println(WiFi.softAPmacAddress());

    if (esp_now_init() == ESP_OK)
    {
      LOGM( "ESPNow Init Success" );

      LOG();
      Serial.print( "WiFi IP: " );
      Serial.print( WiFi.localIP() );

      Serial.print( ", WiFi Netmask: " );
      Serial.print( WiFi.subnetMask() );

      Serial.print( ", WiFi Gateway: " );
      Serial.println( WiFi.gatewayIP() );

      esp_now_register_recv_cb(OnDataRecv);
    }
    else
    {
      LOGM( "ESPNow Init Failed" );
      delay(2000);
      LOGM( "*** RESTART ***" );
      ESP.restart();
    }
  }
}
