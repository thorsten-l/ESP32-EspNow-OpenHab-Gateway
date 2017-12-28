#ifndef __LOG_LOGGER_H__
#define __LOG_LOGGER_H__

#include <Arduino.h>

#define LOG() Serial.printf( "[%d:%12ld] ", xPortGetCoreID(), millis() )
#define LOGM(message) Serial.printf( "[%d:%12ld] %s\n", xPortGetCoreID(), millis(), message )

#endif
