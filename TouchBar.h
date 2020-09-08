#ifndef TouchBar_H
#define TouchBar_H

#include <Arduino.h>

#define Decrement2 0
#define Decrement 63
#define Static 127
#define Increment 192
#define Increment2 255

class TouchBar
{
  private:
    // User configurable variables
    unsigned int PSAddress;
    unsigned int Default; // Default position it will reset to, and load from EEPROM at startup.
    unsigned int Current;
    unsigned int Limit; // This is the max value where it either stops counting, or rolls over. (10000 is default for easy conversion to percentage with a range of 0.00% to 100.00%)
    byte RampDelay; // It's in execution cycles, not ms or us... 0 is invalid!
    byte Resolution; // Output will be incremented/decremented by this amount.
    byte RampResolution; // This is the resoulution for auto adjustment. 0 is invalid!
    byte Flags; // As follows:
    // RollOver - bit 7 --> Set it to true if you want the value to loop infinitely rather then stop at min or max values. (Emulates rotary encoder.) Note that this overrides all other flags!
    // SpringBack - bit 6 --> Set it to true if you want it to reset to default value when you release the pads. (Emulates joystick or pitch wheel that returns to default position when you let it go.)
    // Snap - bit 5 --> Briefly hitting one of the ABC pads it will snap or set ramp to bottom / middle / top of the spectrum. (Quick tapping the A, B or C pads snaps position or target to Min / Default / Max values.)
    // Ramp - bit 4 --> This allows automatic gradual transition from one position to another. (Limits the rate of change, for fast finger swipe, as well as for springback and snap features. Can be used to automatically ramp up/down motor speed, dimm LEDs, etc.)
    byte ABCPads = 0; // This is the input...
    unsigned int TapTimeout; // This controls how fast is a tap on any one of the 3 pads. The lower the number, the faster you should tap.
    byte TwitchSuppressionDelay;
    // Internal variables
    unsigned int RampCounter = 0;
    unsigned int Target;
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
    /* Constructor */
    TouchBar (unsigned int EEPROMAddress); // Start address for 12 bytes of private settings in EEPROM (Applicable to a specific instance.)

    /* Methods */
    // Control Methods
    void SetDefault (unsigned int NewDefault, boolean SaveToEEPROM = false); // Sets Default and optionally saves it to EEPROM
    void SetLimit (unsigned int NewLimit, boolean SaveToEEPROM = false); // Sets Limit and optionally saves it to EEPROM
    void SetResolution (byte NewResolution, boolean SaveToEEPROM = false); // Sets Resolution and optionally saves it to EEPROM
    void SetRampDelay (byte NewRampDelay, boolean SaveToEEPROM = false); // Sets RampDelay and optionally saves it to EEPROM
    void SetRampResolution (byte NewRampResolution, boolean SaveToEEPROM = false); // Sets RampResolution and optionally saves it to EEPROM
    void SetTapTimeout (unsigned int NewTapTimeout, boolean SaveToEEPROM = false); // This needed for pad event.
    void SetTSDelay (byte TSDelay, boolean SaveToEEPROM = false); // Sets TouchSuppression delay for sort of a debouncing...
    void SetFlags (boolean SpringBackFlag, boolean SnapFlag, boolean RampFlag, boolean FlipFlag, boolean SaveToEEPROM = false); // Sets Springback, Snap, Ramp and Flip flags, clears RollOver and optionally saves them to EEPROM
    void SetFlags (boolean RollOverFlag, boolean FlipFlag, boolean SaveToEEPROM = false); // Sets RollOver and Flip flags, clears the others and optionally saves them to EEPROM
    
    // Methods for Monitoring
    unsigned int GetDefault (); // Returns default value.
    unsigned int GetLimit (); // Returns Limit value.
    byte GetRampDelay (); // Returns RampDelay value.
    byte GetRampResolution (); // Returns RampResolution value.
    byte GetResolution (); // Returns Resolution value.
    unsigned int GetTapTimeout (); // Returns TapTimeout value.
    byte TouchBar::GetTSDelay (); // Returns TwitchSuppression value.
    boolean GetSpringBackFlag (); // Returns SpringBack flag status.
    boolean GetRollOverFlag (); // Returns RollOver flag status.
    boolean GetSnapFlag (); // Returns Snap flag status.
    boolean GetRampFlag (); // Returns SlowRamp flag status.
    boolean GetFlipFlag (); // Returns Flip flag status.
    // Operation
    void Init (); // This loads all the defaults from EEPROM. Call it in Setup after initializing the touch library.
    void Restore (); // Restore defaults from EEPROM, without modifying target and position.
    void Update (byte NewValue); // BiTB: 0(LSB) = PadA; 1 = PadB; 2 = PadC; The rest of the bits are ignored.
    void Update (boolean A, boolean B, boolean C); // Another way to do it.
    void SetPosition (unsigned int NewPosition); // Direct control over the position. (You need this if your program has an auto mode, or must be adjustable from software on your computer as well. If that's the case, you wanna adjust the position to aboid sudden jump when you start dragging your finger.)
    void SetTarget (unsigned int NewTarget); // Set target when changing settings temporarily to current position, otherwise it's gonna move immediatly to previously set target when ramp is enabled.
    void Reset (); // Set position or target to default value.
    char PadEvent (); // Returns A, B or C when a single pad was quickly tapped. Returns Z for no event.
    boolean Event (); // Returns true if there's a change.
    unsigned int GetPositionInt (); // Returns current as int.
    float GetPositionFloat (); // Return current as float. (Conveniently it returns the position in % with 2 decimal places if limit set to 10000.)
    unsigned int GetTargetInt (); // Returns current as int.
    float GetTargetFloat (); // Returns Target as float.
}; // <<< ; at the end is important!!!

#endif
