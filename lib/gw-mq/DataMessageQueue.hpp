#ifndef __DATA_MESSAGE_QUEUE_HPP__
#define __DATA_MESSAGE_QUEUE_HPP__

#include <DataDefinition.h>

extern void InitializeDataMessageQueue();
extern void dataMqSend( GwData* data );
extern GwData* dataMqReceive();
extern bool dataMqAvailable();

#endif
