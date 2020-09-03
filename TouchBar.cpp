#include "TouchBar.h"
#include <EEPROM.h>



/* General */

TouchBar::TouchBar (unsigned int EEPROMAddress)
{
   PSAddress = EEPROMAddress;
}

void TouchBar::Init ()
{
  /*
  EEPROM allocation
  Default position --> PSAddress to PSAddress + 1
  Limit --> PSAddress + 2 to PSAddress + 3
  Resolution --> PSAddress + 4
  Ramp delay --> PSAddress + 5
  Ramp resolution --> PSAddress + 6
  Tap timeout --> PSAddress + 7
  Flags --> PSAddress + 8
  Twitch suppression delay --> PSAddress + 9
  
  (Reserved for updates --> PSAddress + 10 to PSAddress + 11 and Flag bits [0-2])
  */
  Default = EEPROM.read (PSAddress) << 8;
  Default = Default + EEPROM.read (PSAddress + 1);
  Current = Default;
  Previous = Default;
  Target = Default;
  Limit = EEPROM.read (PSAddress + 2) << 8;
  Limit = Limit + EEPROM.read (PSAddress + 3);
  Resolution = EEPROM.read (PSAddress + 4);
  RampDelay = EEPROM.read (PSAddress + 5);
  RampResolution = EEPROM.read (PSAddress + 6);
  TapTimeout = EEPROM.read (PSAddress + 7);
  Flags = EEPROM.read (PSAddress + 8);
  TwitchSuppressionDelay = EEPROM.read (PSAddress + 9);
}





/* Settings */
void TouchBar::Reset ()
{
  if (bitRead (Flags, 4) == true)
    Target = Default;
  else
    Current = Default;
}

void TouchBar::SetDefault (unsigned int NewDefault, boolean SaveToEEPROM)
{
  Default = NewDefault;
  if (SaveToEEPROM == true)
  {
    EEPROM.update (PSAddress + 1, NewDefault);
    EEPROM.update (PSAddress, NewDefault >> 8);
  }
}

void TouchBar::SetLimit (unsigned int NewLimit, boolean SaveToEEPROM)
{
  Limit = NewLimit;
  if (SaveToEEPROM == true)
  {
    EEPROM.update (PSAddress + 3, NewLimit);
    EEPROM.update (PSAddress + 2, NewLimit >> 8);
  }
}

void TouchBar::SetResolution (byte NewResolution, boolean SaveToEEPROM)
{
  Resolution = NewResolution;
  if (SaveToEEPROM == true)
  {
    EEPROM.update (PSAddress + 4, NewResolution);
  }
}

void TouchBar::SetRampDelay (byte NewRampDelay, boolean SaveToEEPROM)
{
  RampDelay = NewRampDelay;
  if (SaveToEEPROM == true)
  {
    EEPROM.update (PSAddress + 5, NewRampDelay);
  }
}

void TouchBar::SetRampResolution (byte NewRampResolution, boolean SaveToEEPROM)
{
  RampResolution = NewRampResolution;
  if (SaveToEEPROM == true)
  {
    EEPROM.update (PSAddress + 6, NewRampResolution);
  }
}

void TouchBar::SetTapTimeout (byte NewTapTimeout, boolean SaveToEEPROM)
{
  TapTimeout = NewTapTimeout;
  if (SaveToEEPROM == true)
  {
    EEPROM.update (PSAddress + 7, NewTapTimeout);
  }
}

void TouchBar::SetTSDelay (byte TSDelay, boolean SaveToEEPROM)
{
  TwitchSuppressionDelay = TSDelay;
  if (SaveToEEPROM == true)
  {
    EEPROM.update (PSAddress + 9, TSDelay);
  }
}

void TouchBar::SetFlags (boolean SpringBackFlag, boolean SnapFlag, boolean RampFlag, boolean FlipFlag, boolean SaveToEEPROM)
{
  Flags = 0;
  bitWrite(Flags, 6, SpringBackFlag);
  bitWrite(Flags, 5, SnapFlag);
  bitWrite(Flags, 4, RampFlag);
  bitWrite(Flags, 3, FlipFlag);
  // Bits 7-4 are used the rest of the bits Reserved for additional flags that may be added...
  if (SaveToEEPROM == true)
    EEPROM.update (PSAddress + 8, Flags);
}

void TouchBar::SetFlags (boolean RollOverFlag, boolean FlipFlag, boolean SaveToEEPROM)
{
  Flags = 0;
  bitWrite(Flags, 7, RollOverFlag);
  bitWrite(Flags, 3, FlipFlag);
  // Bits 7-4 are used the rest of the bits Reserved for additional flags that may be added...
  if (SaveToEEPROM == true)
    EEPROM.update (PSAddress + 8, Flags);
}

void TouchBar::SetPosition (unsigned int NewPosition)
{
  Current = NewPosition;
}





/* Get Settings */

unsigned int TouchBar::GetDefault ()
{
  return Default;
}

unsigned int TouchBar::GetLimit ()
{
  return Limit;
}
byte TouchBar::GetResolution ()
{
  return Resolution;
}

byte TouchBar::GetRampDelay ()
{
  return RampDelay;
}

byte TouchBar::GetRampResolution ()
{
  return RampResolution;
}

byte TouchBar::GetTapTimeout ()
{
  return TapTimeout;
}

byte TouchBar::GetTSDelay ()
{
  return TwitchSuppressionDelay;
}

boolean TouchBar::GetRollOverFlag ()
{
  return bitRead (Flags, 7);
}

boolean TouchBar::GetSpringBackFlag ()
{
  return bitRead (Flags, 6);
}

boolean TouchBar::GetSnapFlag ()
{
  return bitRead (Flags, 5);
}

boolean TouchBar::GetRampFlag ()
{
  return bitRead (Flags, 4);
}

boolean TouchBar::GetFlipFlag ()
{
  return bitRead (Flags, 3);
}


/* Input / Output */

void TouchBar::Update (byte NewValue) // This compiles to 30 bytes less then the other Update method.
{
  Shift ();
  
  //TwitchSuppression (NewValue % 8); // This is more beginner friendly...
  TwitchSuppression (NewValue & 0x07);

  Main ();
}

void TouchBar::Update (boolean A, boolean B, boolean C)
{
  Shift ();
  
  byte X = 0;
  //X = (C << 2) + (B << 1) + A; // This looks better
  
  // This does the same, but uses 2 bytes less flash... (I tried like 4 ways of doing this, this is the most compact when compiled, that's 2 instructions less to execute each cycle.)
  X = C;
  X = X << 1;
  X = X + B;
  X = X << 1;
  X = X + A;
  
  TwitchSuppression (X);
  
  Main ();
}

void TouchBar::Shift ()
{
  if (ABCPads != ABCPrevious[0])
  {
    ABCPrevious[2] = ABCPrevious[1];
    ABCPrevious[1] = ABCPrevious[0];
    ABCPrevious[0] = ABCPads;
  }
}

void TouchBar::TwitchSuppression (byte NewValue)
{
  if (TSCounter < 255)
    TSCounter += 1;
  if (NewValue != Raw)
    TSCounter = 0;
  
  if (NewValue != ABCPads && NewValue != 0 && ABCPads != 0 || NewValue ^ ABCPads && TSCounter == TwitchSuppressionDelay)
  {
    Serial.println (NewValue ^ ABCPads, BIN);
    ABCPads = NewValue; // This does the same, compiles to the same size
  }

  Raw = NewValue;
}

boolean TouchBar::Event ()
{
  if (Current != Previous)
    return true;
  else
    return false;
}

char TouchBar::PadEvent ()
{
  if (TapCounter < TapTimeout && ABCPads == 0 && ABCPrevious[0] != 0)
    switch (ABCPrevious[0])
    {
      case 1: return 'A';
      break;;
      case 2: return 'B';
      break;;
      case 4: return 'C';
      break;;
      default: return 'Z';
      break;;
    }
  else
    return 'Z';
}

unsigned int TouchBar::GetPositionInt ()
{
  return Current;
}

float TouchBar::GetPositionFloat ()
{
  return float(Current) / 100;
}

unsigned int TouchBar::GetTargetInt ()
{
  return Target;
}

float TouchBar::GetTargetFloat ()
{
  return float(Target) / 100;
}





/* Execution */

void TouchBar::Main ()
{
  // Swap bits 0 and 2 if necessary
  if (bitRead (Flags, 3) == true)
  {
    boolean X = bitRead(ABCPads, 0);
    bitWrite(ABCPads, 0, bitRead(ABCPads, 2));
    bitWrite(ABCPads, 2, X);
  }
  
  // Tap detection
  if (ABCPads == 1 || ABCPads == 2 || ABCPads == 4 || ABCPads == 0 && ABCPrevious[0] != 0)
    TapCounter += 1;
  else if (ABCPads == 0 && ABCPrevious[0] == 0)
    TapCounter = 0;
  else
    TapCounter = TapTimeout;

  Previous = Current; // This must be before the snap, otherwise snapping works, but does not report the event.

  // Snap
  if (bitRead (Flags, 5) == true && PadEvent() != 'Z')
    switch (PadEvent())
    {
      case 'A': if (bitRead (Flags, 4) == true)
                  Target = 0;
                else
                  Current = 0;
      break;;
      case 'B': Reset();
      break;;
      case 'C': if (bitRead (Flags, 4) == true)
                  Target = Limit;
                else
                  Current = Limit;
      break;;
    }
  
  GetDirection (); // Caluclate direction
  AdjustOutput (); // React...
}

void TouchBar::GetDirection ()
{
  if (ABCPads == 0)
  {
    Direction = Static;
    ABCPrevious[1] = 0;
    ABCPrevious[2] = 0;
    if (bitRead (Flags, 6) == true)
      Reset ();
  }
  else
  {
    /*
    Signals in commenTB (for humans):
    - T - Touched (Rising edge)
    - H - Held (High)
    - R - Released (Falling edge)
    - LU - Left Untouched (Low)
    */
    Direction = Static;
    
    /*
    // This is the old way, it works fine, except the new one is more compact when compiled... This one may be easier to understand for beginners.
    // Light touch scenario (only touching 1-2 pads at a time.)
    // Increment case 1/6 (A=H, B=T, C=LU)
    if (ABCPads == 3 && ABCPrevious[0] == 1)
      Direction = Increment;
    // Increment case 2/6 (A=R, B=H, C=LU)
    if (ABCPads == 2 && ABCPrevious[0] == 3)
      Direction = Increment;
    // Increment case 3/6 (A=LU, B=H, C=T)
    if (ABCPads == 6 && ABCPrevious[0] == 2)
      Direction = Increment;
    // Increment case 4/6 (A=LU, B=R, C=H)
    if (ABCPads == 4 && ABCPrevious[0] == 6)
      Direction = Increment;
    // Increment case 5/6 (A=T, B=LU, C=H)
    if (ABCPads == 5 && ABCPrevious[0] == 4)
      Direction = Increment;
    // Increment case 6/6 (A=H, B=LU, C=R)
    if (ABCPads == 1 && ABCPrevious[0] == 5)
      Direction = Increment;
    
    // Decrement case 1/6 (A=H, B=LU, C=T)
    if (ABCPads == 5 && ABCPrevious[0] == 1)
      Direction = Decrement;
    // Decrement case 2/6 (A=R, B=LU, C=H)
    if (ABCPads == 4 && ABCPrevious[0] == 5)
      Direction = Decrement;
    // Decrement case 3/6 (A=LU, B=T, C=H)
    if (ABCPads == 6 && ABCPrevious[0] == 4)
      Direction = Decrement;
    // Decrement case 4/6 (A=LU, B=H, C=R)
    if (ABCPads == 2 && ABCPrevious[0] == 6)
      Direction = Decrement;
    // Decrement case 5/6 (A=T, B=H, C=LU)
    if (ABCPads == 3 && ABCPrevious[0] == 2)
      Direction = Decrement;
    // Decrement case 6/6 (A=H, B=R, C=LU)
    if (ABCPads == 1 && ABCPrevious[0] == 3)
      Direction = Decrement;
      
    // Hard touch scenario (touching 2-3 pads at a time.) Rrequires checking the 2nd and 3rd previous status(which together shows second previous event) to determine the direction.
    // Previous event won't do cause the cases for forward and backward are all the same, only determined by the the previously active pad, and each pat is first released and then touched again while the other 2 are held, so we need to go back 1 event further with the checks.
    // Case 1/6 (A=H, B=H, C=T)
    if (ABCPads == 7 && ABCPrevious[0] == 3)
    {
      if (ABCPrevious[1] == 7 && ABCPrevious[2] == 5 || ABCPrevious[1] == 1 && ABCPrevious[2] == 0) // Increment (2nd previous event B=T || B=LU)
        Direction = Increment;
      if (ABCPrevious[1] == 7 && ABCPrevious[2] == 6 || ABCPrevious[1] == 2 && ABCPrevious[2] == 0) // Decrement (2nd previous event A=T || A=LU)
        Direction = Decrement;
    }
    // Case 2/6 (A=R, B=H, C=H)
    if (ABCPads == 6 && ABCPrevious[0] == 7)
    {
      if (ABCPrevious[1] == 3 && ABCPrevious[2] == 7 || ABCPrevious[1] == 3 && ABCPrevious[2] == 1) // Increment (2nd previous event B=T || B=LU)
        Direction = Increment;
      if (ABCPrevious[1] == 5 && ABCPrevious[2] == 7 || ABCPrevious[1] == 5 && ABCPrevious[2] == 1) // Decrement (2nd previous event A=T || A=LU)
        Direction = Decrement;
    }
    // Case 3/6 (A=T, B=H, C=H)
    if (ABCPads == 7 && ABCPrevious[0] == 6)
    {
      if (ABCPrevious[1] == 7 && ABCPrevious[2] == 3 || ABCPrevious[1] == 2 && ABCPrevious[2] == 0) // Increment (2nd previous event B=T || B=LU)
        Direction = Increment;
      if (ABCPrevious[1] == 7 && ABCPrevious[2] == 5 || ABCPrevious[1] == 4 && ABCPrevious[2] == 0) // Decrement (2nd previous event A=T || A=LU)
        Direction = Decrement;
    }
    // Case 4/6 (A=H, B=R, C=H)
    if (ABCPads == 5 && ABCPrevious[0] == 7)
    {
      if (ABCPrevious[1] == 6 && ABCPrevious[2] == 7 || ABCPrevious[1] == 6 && ABCPrevious[2] == 2) // Increment (2nd previous event B=T || B=LU)
        Direction = Increment;
      if (ABCPrevious[1] == 3 && ABCPrevious[2] == 7 || ABCPrevious[1] == 3 && ABCPrevious[2] == 2) // Decrement (2nd previous event A=T || A=LU)
        Direction = Decrement;
    }
    // Case 5/6 (A=H, B=T, C=H)
    if (ABCPads == 7 && ABCPrevious[0] == 5)
    {
      if (ABCPrevious[1] == 7 && ABCPrevious[2] == 6 || ABCPrevious[1] == 4 && ABCPrevious[2] == 0) // Increment (2nd previous event B=T || B=LU)
        Direction = Increment;
      if (ABCPrevious[1] == 7 && ABCPrevious[2] == 3 || ABCPrevious[1] == 1 && ABCPrevious[2] == 0) // Decrement (2nd previous event A=T || A=LU)
        Direction = Decrement;
    }
    // Case 6/6 (A=H, B=H, C=R)
    if (ABCPads == 3 && ABCPrevious[0] == 7)
    {
      if (ABCPrevious[1] == 5 && ABCPrevious[2] == 7 || ABCPrevious[1] == 5 && ABCPrevious[2] == 4) // Increment (2nd previous event B=T || B=LU)
        Direction = Increment;
      if (ABCPrevious[1] == 6 && ABCPrevious[2] == 7 || ABCPrevious[1] == 6 && ABCPrevious[2] == 4) // Decrement (2nd previous event A=T || A=LU)
        Direction = Decrement;
    }
    */
    
    // The following does the same as the commented section above, except it compiles to 76-94 bytes less (depending on which Update method is used.).
    // Light touch scenario (only touching 1-2 pads at a time.)
    // Increment case 1/6 (A=H, B=T, C=LU)
    if (!(ABCPads ^ 3 | ABCPrevious[0] ^ 1))
      Direction = Increment;
    // Increment case 2/6 (A=R, B=H, C=LU)
    if (!(ABCPads ^ 2 | ABCPrevious[0] ^ 3))
      Direction = Increment;
    // Increment case 3/6 (A=LU, B=H, C=T)
    if (!(ABCPads ^ 6 | ABCPrevious[0] ^ 2))
      Direction = Increment;
    // Increment case 4/6 (A=LU, B=R, C=H)
    if (!(ABCPads ^ 4 | ABCPrevious[0] ^ 6))
      Direction = Increment;
    // Increment case 5/6 (A=T, B=LU, C=H)
    if (!(ABCPads ^ 5 | ABCPrevious[0] ^ 4))
      Direction = Increment;
    // Increment case 6/6 (A=H, B=LU, C=R)
    if (!(ABCPads ^ 1 | ABCPrevious[0] ^ 5))
      Direction = Increment;
    
    // Decrement case 1/6 (A=H, B=LU, C=T)
    if (!(ABCPads ^ 5 | ABCPrevious[0] ^ 1))
      Direction = Decrement;
    // Decrement case 2/6 (A=R, B=LU, C=H)
    if (!(ABCPads ^ 4 | ABCPrevious[0] ^ 5))
      Direction = Decrement;
    // Decrement case 3/6 (A=LU, B=T, C=H)
    if (!(ABCPads ^ 6 | ABCPrevious[0] ^ 4))
      Direction = Decrement;
    // Decrement case 4/6 (A=LU, B=H, C=R)
    if (!(ABCPads ^ 2 | ABCPrevious[0] ^ 6))
      Direction = Decrement;
    // Decrement case 5/6 (A=T, B=H, C=LU)
    if (!(ABCPads ^ 3 | ABCPrevious[0] ^ 2))
      Direction = Decrement;
    // Decrement case 6/6 (A=H, B=R, C=LU)
    if (!(ABCPads ^ 1 | ABCPrevious[0] ^ 3))
      Direction = Decrement;
    
    // Hard touch scenario (touching 2-3 pads at a time.) Rrequires checking the 2nd and 3rd previous status(which together shows second previous event) to determine the direction.
    // Previous event won't do cause the cases for forward and backward are all the same, only determined by the the previously active pad, and each pat is first released and then touched again while the other 2 are held, so we need to go back 1 event further with the checks.
    // Case 1/6 (A=H, B=H, C=T)
    if (!(ABCPads ^ 7 | ABCPrevious[0] ^ 3))
    {
      if (!(ABCPrevious[1] ^ 7 | ABCPrevious[2] ^ 5) || !(ABCPrevious[1] ^ 1 | ABCPrevious[2] ^ 0)) // Increment (2nd previous event B=T || B=LU)
        Direction = Increment;
      if (!(ABCPrevious[1] ^ 7 | ABCPrevious[2] ^ 6) || !(ABCPrevious[1] ^ 2 | ABCPrevious[2] ^ 0)) // Decrement (2nd previous event A=T || A=LU)
        Direction = Decrement;
    }
    // Case 2/6 (A=R, B=H, C=H)
    if (!(ABCPads ^ 6 | ABCPrevious[0] ^ 7))
    {
      if (!(ABCPrevious[1] ^ 3 | ABCPrevious[2] ^ 7) || !(ABCPrevious[1] ^ 3 | ABCPrevious[2] ^ 1)) // Increment (2nd previous event B=T || B=LU)
        Direction = Increment;
      if (!(ABCPrevious[1] ^ 5 | ABCPrevious[2] ^ 7) || !(ABCPrevious[1] ^ 5 | ABCPrevious[2] ^ 1)) // Decrement (2nd previous event A=T || A=LU)
        Direction = Decrement;
    }
    // Case 3/6 (A=T, B=H, C=H)
    if (!(ABCPads ^ 7 | ABCPrevious[0] ^ 6))
    {
      if (!(ABCPrevious[1] ^ 7 | ABCPrevious[2] ^ 3) || !(ABCPrevious[1] ^ 2 | ABCPrevious[2] ^ 0)) // Increment (2nd previous event B=T || B=LU)
        Direction = Increment;
      if (!(ABCPrevious[1] ^ 7 | ABCPrevious[2] ^ 5) || !(ABCPrevious[1] ^ 4 | ABCPrevious[2] ^ 0)) // Decrement (2nd previous event A=T || A=LU)
        Direction = Decrement;
    }
    // Case 4/6 (A=H, B=R, C=H)
    if (!(ABCPads ^ 5 | ABCPrevious[0] ^ 7))
    {
      if (!(ABCPrevious[1] ^ 6 | ABCPrevious[2] ^ 7) || !(ABCPrevious[1] ^ 6 | ABCPrevious[2] ^ 2)) // Increment (2nd previous event B=T || B=LU)
        Direction = Increment;
      if (!(ABCPrevious[1] ^ 3 | ABCPrevious[2] ^ 7) || !(ABCPrevious[1] ^ 3 | ABCPrevious[2] ^ 2)) // Decrement (2nd previous event A=T || A=LU)
        Direction = Decrement;
    }
    // Case 5/6 (A=H, B=T, C=H)
    if (!(ABCPads ^ 7 | ABCPrevious[0] ^ 5))
    {
      if (!(ABCPrevious[1] ^ 7 | ABCPrevious[2] ^ 6) || !(ABCPrevious[1] ^ 4 | ABCPrevious[2] ^ 0)) // Increment (2nd previous event B=T || B=LU)
        Direction = Increment;
      if (!(ABCPrevious[1] ^ 7 | ABCPrevious[2] ^ 3) || !(ABCPrevious[1] ^ 1 | ABCPrevious[2] ^ 0)) // Decrement (2nd previous event A=T || A=LU)
        Direction = Decrement;
    }
    // Case 6/6 (A=H, B=H, C=R)
    if (!(ABCPads ^ 3 | ABCPrevious[0] ^ 7))
    {
      if (!(ABCPrevious[1] ^ 5 | ABCPrevious[2] ^ 7) || !(ABCPrevious[1] ^ 5 | ABCPrevious[2] ^ 4)) // Increment (2nd previous event B=T || B=LU)
        Direction = Increment;
      if (!(ABCPrevious[1] ^ 6 | ABCPrevious[2] ^ 7) || !(ABCPrevious[1] ^ 6 | ABCPrevious[2] ^ 4)) // Decrement (2nd previous event A=T || A=LU)
        Direction = Decrement;
    }
  }
}

void TouchBar::AdjustOutput ()
{
  if (bitRead (Flags, 4) == true)
  {
    if (Direction == Increment)
    {
      if (Target < Limit - Resolution)
        Target += Resolution;
      else
        Target = Limit;
    }

    if (Direction == Decrement)
    {
      if (Target >= Resolution)
        Target -= Resolution;
      else
        Target = 0;
    }

    if (RampCounter == RampDelay - 1)
    {
      if (Current < Target)
        if (Current < Target - RampResolution)
          Current += RampResolution;
        else
          Current = Target;

      if (Current > Target)
        if (Current > Target + RampResolution)
          Current -= RampResolution;
        else
          Current = Target;
    }

    RampCounter += 1;
    RampCounter %= RampDelay;
  }
  else
  {
    if (Direction == Increment)
    {
      if (bitRead (Flags, 7) == true)
      {
        Current += Resolution;
        if (Current >= Limit)
          Current = 0;
      }
      else
      {
        if (Current < Limit - Resolution)
          Current += Resolution;
        else
          Current = Limit;
      }
    }
    if (Direction == Decrement)
    {
      if (bitRead (Flags, 7) == true)
      {
        Current -= Resolution;
        if (Current >= Limit)
          Current = Limit - Resolution;
      }
      else
      {
        if (Current >= Resolution)
          Current -= Resolution;
        else
          Current = 0;
      }
    }
  }
}
