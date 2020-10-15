#include "TouchBar.h"
#include <EEPROM.h>


boolean UpdateEEPROM (unsigned int Address, byte Data)
{
  if (EEPROM.read(Address) != Data)
  {
    EEPROM.write (Address, Data);
    return 1;
  }
  else
  {
    return 0;
  }
}

#ifdef ESP8266

  void SaveTouchBarConfig (TouchBarCommon *CommonPtr, TouchBarConfig *ConfigPtr, size_t Size, unsigned int EEPROMAddress)
  {
    unsigned int CommitChanges = false;
    CommitChanges += UpdateEEPROM (EEPROMAddress + 1, CommonPtr->TapTimeout);
    CommitChanges += UpdateEEPROM (EEPROMAddress, CommonPtr->TapTimeout >> 8);
    CommitChanges += UpdateEEPROM (EEPROMAddress + 2, CommonPtr->TwitchSuppressionDelay);
    for (int i = 0; i < Size; i++)
    {
      CommitChanges += UpdateEEPROM (EEPROMAddress + 4 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].Default);
      CommitChanges += UpdateEEPROM (EEPROMAddress + 3 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].Default >> 8);
      CommitChanges += UpdateEEPROM (EEPROMAddress + 6 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].Limit);
      CommitChanges += UpdateEEPROM (EEPROMAddress + 5 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].Limit >> 8);
      CommitChanges += UpdateEEPROM (EEPROMAddress + 7 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].Resolution);
      CommitChanges += UpdateEEPROM (EEPROMAddress + 8 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].RampDelay);
      CommitChanges += UpdateEEPROM (EEPROMAddress + 9 + i * sizeof(ConfigPtr[i]), ConfigPtr[i].RampResolution);
      byte Flags = 0;
      bitWrite (Flags, 7, ConfigPtr[i].GetRollOverFlag());
      bitWrite (Flags, 6, ConfigPtr[i].GetSpringBackFlag());
      bitWrite (Flags, 5, ConfigPtr[i].GetSnapFlag());
      bitWrite (Flags, 4, ConfigPtr[i].GetRampFlag());
      bitWrite (Flags, 3, ConfigPtr[i].GetFlipFlag());
      CommitChanges += UpdateEEPROM (EEPROMAddress + 10 + i * sizeof(ConfigPtr[i]), Flags);
    }
    if (CommitChanges > 0)
    {
      EEPROM.commit ();
    }
  }

#else

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

#endif

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
