#include <Arduino.h>
#include <Logger.h>
#include <time.h>
#include "DataMessageQueue.hpp"

#define QUEUE_LENGTH 10

static SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
static GwData queue[QUEUE_LENGTH];
static int newEntry;
static int lastEntry;

void InitializeDataMessageQueue()
{
  LOGM( "Initialize data message queue");
  newEntry = 0;
  lastEntry = 0;
}

void dataMqSend( GwData* data )
{
  xSemaphoreTake( mutex, portMAX_DELAY );

  newEntry %= QUEUE_LENGTH;

  if (((newEntry+1) % QUEUE_LENGTH ) == lastEntry )
  {
    LOGM( "ERROR: data message queue has no more entries!");
  }
  else
  {
    LOG();
    Serial.printf( "sending data into message queue (entry #%d)\n", newEntry );
    memcpy( &queue[newEntry], data, sizeof(gwData));
    time_t now;
    time( &now );
    queue[newEntry].timestamp = now;
    newEntry++;
  }

  xSemaphoreGive( mutex );
}

GwData* dataMqReceive()
{
  xSemaphoreTake( mutex, portMAX_DELAY );

  GwData* retData = 0;

  if ( newEntry != lastEntry )
  {
    lastEntry %= QUEUE_LENGTH;
    retData = &queue[lastEntry];
    LOG();
    Serial.printf( "receiving data from message queue (entry #%d)\n", lastEntry );
    lastEntry++;
  }

  xSemaphoreGive( mutex );
  return retData;
}

bool dataMqAvailable()
{
  bool available;

  xSemaphoreTake( mutex, portMAX_DELAY );
  available = ( newEntry != lastEntry );
  xSemaphoreGive( mutex );

  return available;
}
