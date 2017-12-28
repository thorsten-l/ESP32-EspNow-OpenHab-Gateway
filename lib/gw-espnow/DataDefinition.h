#ifndef __DATA_DEFINITION_H__
#define __DATA_DEFINITION_H__

typedef struct gwData
{
  char name[64];
  int counter;
  unsigned long timestamp;
  double battery;
  char action[128];
} GwData;

#endif
