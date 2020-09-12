#include "TouchBar.h"
#include <EEPROM.h>



void SaveTouchBarConfig (TouchBarCommon *CommonPtr, TouchBarConfig *ConfigPtr, size_t Size, unsigned int EEPROMAddress)
{
  EEPROM.update (EEPROMAddress + 1, CommonPtr->TapTimeout);
  EEPROM.update (EEPROMAddress, CommonPtr->TapTimeout >> 8);
  EEPROM.update (EEPROMAddress + 2, CommonPtr->TwitchSuppressionDelay);
  for (int i = 0; i < Size; i++)
  {
    EEPROM.update (EEPROMAddress + 4 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].Default);
    EEPROM.update (EEPROMAddress + 3 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].Default >> 8);
    EEPROM.update (EEPROMAddress + 6 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].Limit);
    EEPROM.update (EEPROMAddress + 5 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].Limit >> 8);
    EEPROM.update (EEPROMAddress + 7 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].Resolution);
    EEPROM.update (EEPROMAddress + 8 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].RampDelay);
    EEPROM.update (EEPROMAddress + 9 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].RampResolution);
    byte Flags = 0;
    bitWrite (Flags, 7, ConfigPtr[i].GetRollOverFlag());
    bitWrite (Flags, 6, ConfigPtr[i].GetSpringBackFlag());
    bitWrite (Flags, 5, ConfigPtr[i].GetSnapFlag());
    bitWrite (Flags, 4, ConfigPtr[i].GetRampFlag());
    bitWrite (Flags, 3, ConfigPtr[i].GetFlipFlag());
    EEPROM.update (EEPROMAddress + 10 + i * sizeof(ConfigPtr[i]), Flags);
  }
}

void LoadTouchBarConfig (TouchBarCommon *CommonPtr, TouchBarConfig *ConfigPtr, size_t Size, unsigned int EEPROMAddress)
{
  CommonPtr->TapTimeout = EEPROM.read (EEPROMAddress) << 8;
  CommonPtr->TapTimeout = CommonPtr->TapTimeout + EEPROM.read (EEPROMAddress + 1);
  CommonPtr->TwitchSuppressionDelay = EEPROM.read (EEPROMAddress + 2);
  for (int i = 0; i < Size; i++)
  {
    ConfigPtr[i].Default = EEPROM.read (EEPROMAddress + 3 + i * sizeof(ConfigPtr[i])) << 8;
    ConfigPtr[i].Default = ConfigPtr[i].Default + EEPROM.read (EEPROMAddress + 4 + i * sizeof(ConfigPtr[i]));
    ConfigPtr[i].Limit = EEPROM.read (EEPROMAddress + 5 + i * sizeof(ConfigPtr[i])) << 8;
    ConfigPtr[i].Limit = ConfigPtr[i].Limit + EEPROM.read (EEPROMAddress + 6 + i * sizeof(ConfigPtr[i]));
    ConfigPtr[i].Resolution = EEPROM.read (EEPROMAddress + 7 + i * sizeof(ConfigPtr[i]));
    ConfigPtr[i].RampDelay = EEPROM.read (EEPROMAddress + 8 + i * sizeof(ConfigPtr[i]));
    ConfigPtr[i].RampResolution = EEPROM.read (EEPROMAddress + 9 + i * sizeof(ConfigPtr[i]));
    byte Flags = EEPROM.read (EEPROMAddress + 10 + i * sizeof(ConfigPtr[i]));
    if (bitRead(Flags, 7) == true)
      ConfigPtr[i].SetFlags(bitRead(Flags, 7), bitRead(Flags, 3));
    else
      ConfigPtr[i].SetFlags(bitRead(Flags, 6), bitRead(Flags, 5), bitRead(Flags, 4), bitRead(Flags, 3));
  }
}
