#include "GwSdCard.hpp"
#include <Logger.h>
#include <PrivateConfig.h>

#include <FS.h>
#include <SD_MMC.h>

bool sdCardInitialized;

void InitializeSdCard()
{
  sdCardInitialized = false;

  if(!SD_MMC.begin())
  {
    LOGM("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE)
  {
    LOGM("No SD_MMC card attached");
    return;
  }

  LOG();
  Serial.print("SD_MMC Card Type: ");

  if(cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else
  if(cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else
  if(cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }

  LOG();
  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

  LOG();
  Serial.printf("SD_MMC Card Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
  LOG();
  Serial.printf("SD_MMC Card Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));

  sdCardInitialized = true;
}


void listDir(fs::FS &fs, const char * dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);

  if(!root)
  {
    Serial.println("Failed to open directory");
    return;
  }

  if(!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();

  while(file)
  {
    if(file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());

      if(levels)
      {
        listDir(fs, file.name(), levels -1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }

    file = root.openNextFile();
  }
}

void TestSdCard()
{
  if ( sdCardInitialized )
  {
    LOG();
    Serial.println();
    listDir(SD_MMC, "/", 0);
    Serial.println();
  }
  else
  {
    LOGM( "SD Card NOT initialized" );
  }
}
