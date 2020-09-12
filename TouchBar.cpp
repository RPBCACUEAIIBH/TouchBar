#include "TouchBar.h"



/* General */
TouchBar::TouchBar (TouchBarCommon *CommonPtr, TouchBarConfig *ConfigPtr)
{
  Common = CommonPtr;
  Config = ConfigPtr;
  Current = Config->Default;
  Previous = Config->Default;
  Target = Config->Default;
}

void TouchBar::Reconfigure (TouchBarConfig *ConfigPtr)
{
  Config = ConfigPtr;
  Target = Current;
}



/* Settings */
void TouchBar::Reset ()
{
  if (Config->GetRampFlag() == true)
    Target = Config->Default;
  else
    Current = Config->Default;
}

void TouchBar::SetPosition (unsigned int NewPosition)
{
  Current = NewPosition;
}

void TouchBar::SetTarget (unsigned int NewTarget)
{
  Target = NewTarget;
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
  
  // This does the same, but uses 2 bytes less flash... (I tried like 4 ways of doing this, this is the most compact when compiled, that's 2 instructions less to execute each cycle.)
  X = C;
  X = X << 1;
  X = X + B;
  X = X << 1;
  X = X + A;

  //X = (C << 2) + (B << 1) + A; // This looks better and does the same but for some reason not as compact then compiled.
  
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
  
  if (NewValue != ABCPads && NewValue != 0 && ABCPads != 0 || NewValue ^ ABCPads && TSCounter == Common->TwitchSuppressionDelay)
    ABCPads = NewValue; // This does the same, compiles to the same size

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
  if (TapCounter < Common->TapTimeout && ABCPads == 0 && ABCPrevious[0] != 0)
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
  if (Config->GetFlipFlag() == true)
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
    TapCounter = Common->TapTimeout;

  Previous = Current; // This must be before the snap, otherwise snapping works, but does not report the event.

  // Snap
  if (Config->GetSnapFlag() == true && PadEvent() != 'Z')
    switch (PadEvent())
    {
      case 'A': if (Config->GetRampFlag() == true)
                  Target = 0;
                else
                  Current = 0;
      break;;
      case 'B': Reset();
      break;;
      case 'C': if (Config->GetRampFlag() == true)
                  Target = Config->Limit;
                else
                  Current = Config->Limit;
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
    if (Config->GetSpringBackFlag() == true)
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
    
    // The following does the same as the commented section above, except it compiles to 76-94 bytes less (depending on which Update method is used.).
    // Light touch scenario (only touching 1-2 pads at a time.)
    // Update: The condition for skipping a state change was added as an afterthought, which sped up the input significantly, but it can't be optimized the way it was before.
    // Increment case 1/6
    if (ABCPads == 3)
    {
      if (ABCPrevious[0] == 1) // Normal (A=H, B=T, C=LU)
        Direction = Increment;
      if (ABCPrevious[0] == 5) // Skiping (A=H, B=T, C=R)
        Direction = Increment2;
    }
    // Increment case 2/6
    if (ABCPads == 2)
    {
      if (ABCPrevious[0] == 3) // Normal (A=R, B=H, C=LU)
        Direction = Increment;
      if (ABCPrevious[0] == 1) // Skiping (A=R, B=T, C=LU)
        Direction = Increment2;
    }
    // Increment case 3/6
    if (ABCPads == 6)
    {
      if (ABCPrevious[0] == 2) // Normal (A=LU, B=H, C=T)
        Direction = Increment;
      if (ABCPrevious[0] == 3) // Skiping (A=R, B=H, C=T)
        Direction = Increment2;
    }
    // Increment case 4/6
    if (ABCPads == 4)
    {
      if (ABCPrevious[0] == 6) // Normal (A=LU, B=R, C=H)
        Direction = Increment;
      if (ABCPrevious[0] == 2) // Skiping (A=LU, B=R, C=T)
        Direction = Increment2;
    }
    // Increment case 5/6
    if (ABCPads == 5)
    {
      if (ABCPrevious[0] == 4) // Normal (A=T, B=LU, C=H)
        Direction = Increment;
      if (ABCPrevious[0] == 6) // Skiping (A=T, B=R, C=H)
        Direction = Increment2;
    }
    // Increment case 6/6
    if (ABCPads == 1)
    {
      if (ABCPrevious[0] == 5) // Normal (A=H, B=LU, C=R)
        Direction = Increment;
      if (ABCPrevious[0] == 4) // Skiping (A=T, B=LU, C=R)
        Direction = Increment2;
    }

    // Decrement case 1/6
    if (ABCPads == 5)
    {
      if (ABCPrevious[0] == 1) // Normal (A=H, B=LU, C=T)
        Direction = Decrement;
      if (ABCPrevious[0] == 3) // Skiping (A=H, B=R, C=T)
        Direction = Decrement2;
    }
    // Decrement case 2/6
    if (ABCPads == 4)
    {
      if (ABCPrevious[0] == 5) // Normal (A=R, B=LU, C=H)
        Direction = Decrement;
      if (ABCPrevious[0] == 1) // Skiping (A=R, B=LU, C=T)
        Direction = Decrement2;
    }
    // Decrement case 3/6
    if (ABCPads == 6)
    {
      if (ABCPrevious[0] == 4) // Normal (A=LU, B=T, C=H)
        Direction = Decrement;
      if (ABCPrevious[0] == 5) // Skiping (A=R, B=T, C=H)
        Direction = Decrement2;
    }
    // Decrement case 4/6
    if (ABCPads == 2)
    {
      if (ABCPrevious[0] == 6) // Normal (A=LU, B=H, C=R)
        Direction = Decrement;
      if (ABCPrevious[0] == 4) // Skiping (A=LU, B=T, C=R)
        Direction = Decrement2;
    }
    // Decrement case 5/6
    if (ABCPads == 3)
    {
      if (ABCPrevious[0] == 2) // Normal (A=T, B=H, C=LU)
        Direction = Decrement;
      if (ABCPrevious[0] == 6) // Skiping (A=T, B=H, C=R)
        Direction = Decrement2;
    }
    // Decrement case 6/6
    if (ABCPads == 1)
    {
      if (ABCPrevious[0] == 3) // Normal (A=H, B=R, C=LU)
        Direction = Decrement;
      if (ABCPrevious[0] == 2) // Skiping (A=T, B=R, C=LU)
        Direction = Decrement2;
    }
    
    /*
    // Update: The condition for skipping can only be implemented for light touch, as the twitch suppression would filter out fast change on the same pin anyway, so it would not make sense.
    // (It is hard to drag your finger so fast that if would skipp state change when you're pushing it hard on the surface, so it has very limited usefulness anyway.)
    
    // This commented section works as the next optimized section, however it's kept cause it may be more understandable for beginners. ...and me. :P I don't wanna stare at it and wonder how the hack did I do it in some time... ;)
    
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
  if (Config->GetRampFlag() == true)
  {
    if (Direction > Static)
    {
      if (Target < Config->Limit - Config->Resolution && Direction == Increment)
        Target += Config->Resolution;
      else
      {
        if (Target < Config->Limit - Config->Resolution && Direction == Increment2)
          Target += Config->Resolution * 2;
        else
          Target = Config->Limit;
      }
    }
    if (Direction < Static)
    {
      if (Target >= Config->Resolution && Direction == Decrement)
        Target -= Config->Resolution;
      else
      {
        if (Target >= Config->Resolution && Direction == Decrement2)
          Target -= Config->Resolution * 2;
        else
          Target = 0;
      }
    }

    if (RampCounter == Config->RampDelay - 1)
    {
      if (Current < Target)
        if (Current < Target - Config->RampResolution)
          Current += Config->RampResolution;
        else
          Current = Target;

      if (Current > Target)
        if (Current > Target + Config->RampResolution)
          Current -= Config->RampResolution;
        else
          Current = Target;
    }

    RampCounter += 1;
    RampCounter %= Config->RampDelay;
  }
  else
  {
    if (Direction > Static)
    {
      if (Config->GetRollOverFlag() == true)
      {
        if (Direction == Increment)
          Current += Config->Resolution;
        if (Direction == Increment2)
          Current += Config->Resolution * 2;
        Current %= Config->Limit;
      }
      else
      {
        if (Current < Config->Limit - Config->Resolution && Direction == Increment)
          Current += Config->Resolution;
        else
        {
          if (Current < Config->Limit - Config->Resolution * 2 && Direction == Increment2)
            Current += Config->Resolution * 2;
          else
            Current = Config->Limit;
        }
      }
    }
    if (Direction < Static)
    {
      if (Config->GetRollOverFlag() == true)
      {
        byte X = 0;
        if (Direction == Decrement)
          X = Config->Resolution;
        if (Direction == Decrement2)
          X = Config->Resolution * 2;
        if (Current - X >= Config->Limit)
        {
          X -= Current;
          Current = Config->Limit - X;
        }
        else
          Current -= X;
      }
      else
      {
        if (Current >= Config->Resolution && Direction == Decrement)
          Current -= Config->Resolution;
        else
        {
          if (Current >= Config->Resolution * 2 && Direction == Decrement2)
            Current -= Config->Resolution * 2;
          else
            Current = 0;
        }
      }
    }
  }
}
