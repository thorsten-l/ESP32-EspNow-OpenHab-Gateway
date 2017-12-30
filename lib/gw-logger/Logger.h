#ifndef __LOG_LOGGER_H__
#define __LOG_LOGGER_H__

#include <Arduino.h>

#define LOG() Serial.printf( "[%d:%12ld] ", xPortGetCoreID(), millis() )
#define LOGM(message) Serial.printf( "[%d:%12ld] %s\n", xPortGetCoreID(), millis(), message )
#define LOGF( FMTMSG, ... ) Serial.printf( "[%d:%12ld] " FMTMSG "\n", xPortGetCoreID(), millis(), ##__VA_ARGS__ )

#endif
