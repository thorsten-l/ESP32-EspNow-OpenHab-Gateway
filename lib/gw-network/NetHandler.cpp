#include "NetHandler.hpp"
#include <Logger.h>
#include <PrivateConfig.h>

bool eth_connected = false;

void NetEventHandler(WiFiEvent_t event)
{
  LOG();

  switch (event) {

    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname(ETH_HOSTNAME);
      break;

    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;

    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;

    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;

    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;

    case SYSTEM_EVENT_AP_START:
      Serial.println( "WiFi AP Started" );
      break;

    default:
      Serial.print("Event : ");
      Serial.println( event );
      break;
  }
}
