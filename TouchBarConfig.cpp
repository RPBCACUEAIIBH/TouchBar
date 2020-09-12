#include "TouchBar.h"



/* Configure Private */
void TouchBarConfig::SetFlags (boolean SpringBackFlag, boolean SnapFlag, boolean RampFlag, boolean FlipFlag)
{
  Flags = 0;
  bitWrite(Flags, 6, SpringBackFlag);
  bitWrite(Flags, 5, SnapFlag);
  bitWrite(Flags, 4, RampFlag);
  bitWrite(Flags, 3, FlipFlag);
}

void TouchBarConfig::SetFlags (boolean RollOverFlag, boolean FlipFlag)
{
  Flags = 0;
  bitWrite(Flags, 7, RollOverFlag);
  bitWrite(Flags, 3, FlipFlag);
}



/* Get Private */
boolean TouchBarConfig::GetRollOverFlag ()
{
  return bitRead (Flags, 7);
}

boolean TouchBarConfig::GetSpringBackFlag ()
{
  return bitRead (Flags, 6);
}

boolean TouchBarConfig::GetSnapFlag ()
{
  return bitRead (Flags, 5);
}

boolean TouchBarConfig::GetRampFlag ()
{
  return bitRead (Flags, 4);
}

boolean TouchBarConfig::GetFlipFlag ()
{
  return bitRead (Flags, 3);
}
