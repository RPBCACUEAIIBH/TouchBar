/*
The TouchBar library is an engine designed to take an input, of 3 bits (first 3 bits of a byte such as the output of my TouchLib library or the Adafruit_MPR121 library.) and interpret it as a touch bar.


Hardware requirements:
- ESP8266 (I'm using ESP12F a "Generic ESP8266 Module")
- MPR 121 touch module hooked up to i2c via the level shifter(cause it's a 3.3V device)
- Touch bar hooked up to 0,1 and 2 touch touch inputs of the MPR121 module according to the provided documentation.


Libraries requirements:
- ESP8266 software (in Tools/Board/Board Manager...)
- Adafruit_MPR121 (or similar... not included, you need to install it separately!)
- Wire (required by Adafruit_MPR121, Should be included with your IDE)
- TouchBar (This one...)
- EEPROM (Required by TouchBar, Should be included with your IDE)


Skill requirements:
- You need to have basic arduino skills. (Understanding how to interface 5V and 3.3V modules, hooking up and testing an i2c device with level shifter, soldering, understanding arduino code, installing and using libraries, etc. This is a library of source code and CAD files with an example sketch not a tutorial, so I won't explain everything here.)
- You either need to make a PCB or order one! (The touch bar itself is basically a footprint you have to print on a PCB, it's a custom design, you can't really buy it.)


Note:
- If you wonder what the touch bar should look like, there's a Ki-CAD folder included in the library, containing sybmols and footprints you can use to print one on a PCB.
- If you find this useful, please consider donationg: http://osrc.rip/Support.html
- If you wanna make the most out of this library please read the documentatuon!
*/

#include <Adafruit_MPR121.h>
#include <TouchBar.h>
#include <EEPROM.h>

// MPR121 Driver Object
Adafruit_MPR121 TouchModule = Adafruit_MPR121();
int CTouched = 0;
int PTouched;

// TouchBar objects
// The Common object applies to every TouchBar object. These are tuning options, and should be left untouched unless your arduino runs on other then 16MHz, or your program is so long, it's getting slow...
TouchBarCommon Common = {140, 20}; // unsigned int TapTimeout, byte TwitchSuppressionDelay
/*
  Note:
  - TapTimeout - may not be obvious, if you rest your finger on the touch bar, but you change your mind and don't wanna ajust it, any you're only touching 1 of the pads, it may interpret it as a tap,
    which could lead to trouble when snap functionality is enabled. TapTimeout is designed to prevent that. (It works, but not exactly foolproof, if you go crazy on the touchbar it may snap every
    once in a while. For critical application where an accidental tap may lead to serious consequences, I'd recommend only enabling snap functionality in the special mode when the function pad is held!)
  - TwitchSuppressionDelay - Selective debouncing, which suppresses oscillation on a pad, but passes change of state on any other pad then the last active.
  - Both values refer to cycles of execution, not milliseconds or microseconds, thus frequency and long loops in your program may affect it!
*/

// Any of the config objects are modes of operation for the TouchBar objects, and can be used by one or more TouchBar objects at once.
TouchBarConfig Config[2];

TouchBar TB (&Common, &Config[0]); // It takes: TouchBarCommon *CommonPtr, TouchBarConfig *ConfigPtr

// Variables
boolean PreviousFunctionState = false;
unsigned int PreviousTarget;

void setup ()
{
  Serial.begin(115200); // <<< 9600 is slow... (...at least for me. :P)
  Serial.println ();
  Serial.println ();

  EEPROM.begin (1024); // This is required for ESP8266!
  Wire.begin(2,0);// Default SDA and SCL pins for ESP8266 are 4 and 5, however I prefer using 2(SDA) and 0(SCL) both of which must be pulled HIGH anyway for bootmode...

  // First initialize the MPR121 library
  if (!TouchModule.begin(0x5A))
  {
    Serial.println(F("MPR121 not found!"));
    while (1);
  }

  /* TouchBar */
  // Config[0] - normal settings
  Config[0].Default = 5000; // Valid range: 0 to Limit
  Config[0].Limit = 10000; // Valid range: Limit > 3 && Limit < 65535 && Limit > Resolution && Limit > RampResolution
  Config[0].Resolution = 100; // Valid range: Resolution > 0 && Resolution < Limit
  Config[0].RampDelay = 100; // This depends on execution speed as well. It's defined in cycles of executon not ms or us... Valid range: 0 to 255
  Config[0].RampResolution = 25; // Valid range: RampResolution > 0 && RampResolution < Limit
  // Use only one of the following 2 lines! It is an overloaded method, one of them should be commented!
  Config[0].SetFlags(false, true, false, false); // It takes: SpringBackFlag, SnapFlag, RampFlag, FlipFlag; SpringBack overrides Snap and thus don't work together, every other combination should be fine.
  //Config[0].SetFlags (true, false); // it takes: RollOverFlag, FlipFlag; RollOver doesn't work with anything but flip, even worse with Ramp on it suddenly changes direction when you pass the limit...

  // Config[1] - special settings(active when the function pad is held)
  Config[1].Default = 5000; // Valid range: 0 to Limit
  Config[1].Limit = 10000; // Valid range: Limit > 3 && Limit < 65535 && Limit > Resolution && Limit > RampResolution
  Config[1].Resolution = 350; // Valid range: Resolution > 0 && Resolution < Limit
  Config[1].RampDelay = 100; // This depends on execution speed as well. It's defined in cycles of executon not ms or us... Valid range: 0 to 255
  Config[1].RampResolution = 25; // Valid range: RampResolution > 0 && RampResolution < Limit
  // Use only one of the following 2 lines! It is an overloaded method, one of them should be commented!
  Config[1].SetFlags(false, true, true, false); // It takes: boolean SpringBackFlag, boolean SnapFlag, boolean RampFlag, boolean FlipFlag
  //Config[1].SetFlags (true, false); // it takes: boolean RollOverFlag, boolean FlipFlag
  /*
    Note:
    - You can either have limited range, and fancy features, or unlimited range, and no fancy features...
    - Snap does nothing when springback is also enabled.

    Play with the presets first, and find out what they do.
  */

  // Initialization
  TB.SetPosition(Config[0].Default); // This sets target to default. If you don't call this it's gonna start at 0 regardless. I could make an Init() method for this, but it's already a 1 liner anyway.
  // Calling TB.Reset() to initialize it also works, but if Ramp flag is set, then it sets Target rather then Position, and ramps up to default(which can be used to soft start something).

  // This is an easy way to save/load the settings to/from EEPROM (Optional, in v2.0 and newer it's no longger built into the touchbar class.)
  // Both take: TouchBarCommon*(pointer to Common object), TouchBarConfig(the entire Config object array), sizeof(Config)/sizeof(Config[0]), EEPROMAddress
  // Requires: sizeof(Common) + sizeof(Config) bytes of EEPROM
  // Without the use of EEPROM, it compiles to 550 bytes less.
  //SaveTouchBarConfig (&Common, Config, sizeof(Config)/sizeof(Config[0]), 0); // Can be left uncommented, it uses EEPROM.update()
  //LoadTouchBarConfig (&Common, Config, sizeof(Config)/sizeof(Config[0]), 0);

  // Report (without this report it compiles to 564 bytes less)
  Serial.print (F("Reporting Touch Bar status:"));
  Serial.print (F("\n  TapTimeout = "));
  Serial.print (Common.TapTimeout);
  Serial.print (F("\n  TSDelay = "));
  Serial.print (Common.TwitchSuppressionDelay);
  Serial.println ();
  Serial.print (F("\n  Default = "));
  Serial.print (Config[0].Default);
  Serial.print (F("\n  Limit = "));
  Serial.print (Config[0].Limit);
  Serial.print (F("\n  Resolution = "));
  Serial.print (Config[0].Resolution);
  Serial.print (F("\n  RampDelay = "));
  Serial.print (Config[0].RampDelay);
  Serial.print (F("\n  RampResolution = "));
  Serial.print (Config[0].RampResolution);
  Serial.print (F("\n  RollOverFlag = "));
  Serial.print (Config[0].GetRollOverFlag ());
  Serial.print (F("\n  SpringBackFlag = "));
  Serial.print (Config[0].GetSpringBackFlag ());
  Serial.print (F("\n  SnapFlag = "));
  Serial.print (Config[0].GetSnapFlag ());
  Serial.print (F("\n  RampFlag = "));
  Serial.print (Config[0].GetRampFlag ());
  Serial.print (F("\n  FlipFlag = "));
  Serial.print (Config[0].GetFlipFlag ());
  Serial.println ();
  Serial.println ();

  Serial.println(F("Initialization done!"));
}

void loop ()
{
  PreviousTarget = TB.GetTargetInt();
  // Read pads
  byte AllPads = TouchModule.touched(); // The adafruit library actually returns a 12 bit value, but the TouchBar only uses the first 3 bits, and the 4th is the function pad.
  /* Function Pad */
  boolean X = bitRead (AllPads, 3); // Extract function pad state
  if (X == HIGH && PreviousFunctionState == LOW)
  {
    Serial.println (F("The Function pad got touched!"));
    // This should temporarily enable ramp mode, as long as you hold the function pad. (This presents 2 new features. SetTaget() and Restore())
    TB.Reconfigure (&Config[1]);
  }
  if (X == LOW && PreviousFunctionState == HIGH)
  {
    Serial.println (F("The Function pad got released!"));
    // Restore previous settings from EEPROM without reseting current position and target.
    TB.Reconfigure (&Config[0]);
  }
  PreviousFunctionState = X;

  /* TouchBar */
  PreviousTarget = TB.GetTargetInt();
  // Feed the pads to TouchBar.
  TB.Update (AllPads); // Takes a byte, and uses the first 3 bits

  // Get tap.
  if (TB.PadEvent() != 'Z')
  {
    Serial.print (F("Tapped the "));
    Serial.print (TB.PadEvent());
    Serial.println (F(" pad."));
  }

  // Get target and position.
  if (TB.GetTargetInt () != PreviousTarget)
  {
    Serial.print (F("Target set to "));
    Serial.print (TB.GetTargetFloat()); // Displaying target position in percentage.
    Serial.println (F("%"));
  }
  if (TB.Event() == true)
  {
    Serial.print (F("CPos: "));
    Serial.print (TB.GetPositionFloat()); // Displaying current position in percentage.
    Serial.println (F("%"));
  }
}
