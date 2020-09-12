#ifndef TouchBar_H
#define TouchBar_H

#include <Arduino.h>

#define Decrement2 0
#define Decrement 63
#define Static 127
#define Increment 192
#define Increment2 255

class TouchBarCommon // These depend on execution speed and should be the same for each touchbar instance, although may require some tuning...
{
  public:
  unsigned int TapTimeout; // This controls how fast is a tap on any one of the 3 pads. The lower the number, the faster you should tap.
  byte TwitchSuppressionDelay;
}; // <<< ; at the end is important!!!

class TouchBarConfig // These are settings specific to a touch bar instance and/or mode of operation...
{
  private:
    byte Flags; // As follows:
    // RollOver - bit 7 --> Set it to true if you want the value to loop infinitely rather then stop at min or max values. (Emulates rotary encoder.) Note that this overrides all other flags!
    // SpringBack - bit 6 --> Set it to true if you want it to reset to default value when you release the pads. (Emulates joystick or pitch wheel that returns to default position when you let it go.)
    // Snap - bit 5 --> Briefly hitting one of the ABC pads it will snap or set ramp to bottom / middle / top of the spectrum. (Quick tapping the A, B or C pads snaps position or target to Min / Default / Max values.)
    // Ramp - bit 4 --> This allows automatic gradual transition from one position to another. (Limits the rate of change, for fast finger swipe, as well as for springback and snap features. Can be used to automatically ramp up/down motor speed, dimm LEDs, etc.)
    // Flags are easy to mess up, therefore they are configured with methods.

  public:
    unsigned int Default; // Valid range: 0 to Limit
    unsigned int Limit; // Valid range: Limit > 3 && Limit < 65535 && Limit > Resolution && Limit > RampResolution
    byte Resolution; // Valid range: Resolution > 0 && Resolution < Limit
    byte RampDelay; // This depends on execution speed as well. It's defined in cycles of executon not ms or us... Valid range: 0 to 255
    byte RampResolution; // Valid range: RampResolution > 0 && RampResolution < Limit
    // Setting everything with methods would also require getting everthing with methods, which would unnecessarily complicate stuff, so it's public and the user should take care to operate it within valid ranges.

    /* Constructor(s) */
    TouchBarConfig (){};

    /* Methods */
    void SetFlags (boolean RollOverFlag, boolean FlipFlag);
    void SetFlags (boolean SpringBackFlag, boolean SnapFlag, boolean RampFlag, boolean FlipFlag);
    boolean GetSpringBackFlag ();
    boolean GetRollOverFlag ();
    boolean GetSnapFlag ();
    boolean GetRampFlag ();
    boolean GetFlipFlag ();
    
}; // <<< ; at the end is important!!!

class TouchBar
{
  private:
    // Input/Output variables
    TouchBarCommon *Common;
    TouchBarConfig *Config;
    unsigned int Current;
    unsigned int Target;
    byte ABCPads = 0;
    // Internal variables
    unsigned int RampCounter = 0;
    unsigned int Previous;
    unsigned int TapCounter;
    byte ABCPrevious[3] = {0, 0, 0};
    byte Direction = Static;
    byte Raw = 0;
    byte TSCounter = 0;

    // Private methods
    void Shift ();
    void Main ();
    void GetDirection ();
    void AdjustOutput ();
    void TwitchSuppression (byte NewValue);

  public:
    // Constructor
    TouchBar (TouchBarCommon *CommonPtr, TouchBarConfig *ConfigPtr);

    // Control Methods
    void Reconfigure (TouchBarConfig *ConfigPtr);
    // Operation
    
    void Update (byte NewValue); // BiTB: 0(LSB) = PadA; 1 = PadB; 2 = PadC; The rest of the bits are ignored.
    void Update (boolean A, boolean B, boolean C); // Another way to do it.
    void SetPosition (unsigned int NewPosition); // Direct control over the position.
    void SetTarget (unsigned int NewTarget); // Set target when changing settings temporarily to current position, otherwise it's gonna move immediatly to previously set target when ramp is enabled.
    void Reset (); // Set position or target to default value.
    char PadEvent (); // Returns A, B or C when a single pad was quickly tapped. Returns Z for no event.
    boolean Event (); // Returns true if there's a change.
    unsigned int GetPositionInt (); // Returns current as int.
    float GetPositionFloat (); // Return current as float. (Conveniently it returns the position in % with 2 decimal places if limit set to 10000.)
    unsigned int GetTargetInt (); // Returns current as int.
    float GetTargetFloat (); // Returns Target as float.
}; // <<< ; at the end is important!!!

void SaveTouchBarConfig (TouchBarCommon *CommonPtr, TouchBarConfig *ConfigPtr, size_t Size, unsigned int EEPROMAddress);
void LoadTouchBarConfig (TouchBarCommon *CommonPtr, TouchBarConfig *ConfigPtr, size_t Size, unsigned int EEPROMAddress);

#endif
