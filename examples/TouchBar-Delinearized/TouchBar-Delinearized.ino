/*
The TouchBar library is an engine designed to take an input, of 3 bits (first 3 bits of a byte such as the output of my TouchLib library or the Adafruit_MPR121 library) and interpret it as a touch bar.


Hardware requirements:
- Arduino running at 16MHz (Any type should do. 16MHz is important cause delays are based on cycles of execution not ms / us, 8MHz will work but require different settings. Mine is pro-mini 5V, 16MHz version.)
- Touch bar hooked up to 3 digital inputs, each with an external 1M pullup resistor.


Libraries requirements:
- TouchLib (Available among my github repositories: https://github.com/RPBCACUEAIIBH/TouchLib)
- TouchBar (This one...)
- Delinearizer (Available among my github repositories: https://github.com/RPBCACUEAIIBH/Delinearizer)
- EEPROM (Required by TouchBar, Should be included with your IDE, you're not required to use the EEPROM, it's an option you can enable at setup().)


Skill requirements:
- You need to have basic arduino skills. (Understanding how to interface 5V and 3.3V modules, hooking up and testing an i2c device with level shifter, soldering, understanding arduino code, installing and using libraries, etc. This is a library of source code and CAD files with an example sketch not a tutorial, so I won't explain everything here.)
- You either need to make a PCB or order one! (The touch bar itself is basically a footprint you have to print on a PCB, it's a custom design, you can't really buy it.)


Note:
- If you wonder what the touch bar should look like, there's a Ki-CAD folder included in the library, containing sybmols and footprints you can use to print one on a PCB.
- If you find this useful, please consider donationg: http://osrc.rip/Support.html
- If you wanna make the most out of this library please read the documentatuon!
*/

#include <TouchLib.h>
#include <TouchBar.h>
#include <Delinearizer.h>

DigitalTouch TInA(A0);
DigitalTouch TInB(A1);
DigitalTouch TInC(A2);
boolean PreviousFunctionState = false;

// TouchBar objects
TouchBarCommon Common = {320, 60}; // unsigned int TapTimeout, byte TwitchSuppressionDelay
TouchBarConfig Config[1];
TouchBar TB (&Common, &Config[0]); // It takes: TouchBarCommon *CommonPtr, TouchBarConfig *ConfigPtr

// Variables
unsigned int PreviousTarget;

void setup()
{
  Serial.begin (115200);
  
  /* TouchLib */
  while (TInA.Calibrate())
    Serial.println (F("Calibration for Touch Input A failed! Retrying..."));
  while (TInB.Calibrate())
    Serial.println (F("Calibration for Touch Input B failed! Retrying..."));
  while (TInC.Calibrate())
    Serial.println (F("Calibration for Touch Input C failed! Retrying..."));

  /* TouchBar */
  // Config[0] - normal settings
  Config[0].Default = 0; // Valid range: 0 to Limit
  Config[0].Limit = 10000; // Valid range: Limit > 3 && Limit < 65535 && Limit > Resolution && Limit > RampResolution
  Config[0].Resolution = 100; // Valid range: Resolution > 0 && Resolution < Limit
  Config[0].RampDelay = 100; // This depends on execution speed as well. It's defined in cycles of executon not ms or us... Valid range: 0 to 255
  Config[0].RampResolution = 25; // Valid range: RampResolution > 0 && RampResolution < Limit
  Config[0].SetFlags(false, true, true, false); // It takes: SpringBackFlag, SnapFlag, RampFlag, FlipFlag; SpringBack overrides Snap and thus don't work together, every other combination should be fine.
  /*
    Note:
    - You can either have limited range, and fancy features, or unlimited range, and no fancy features...
    - Snap does nothing when springback is also enabled.

    Since the Config[0].Default is set to 0 (which makes generating values for a grap easy), and tapping the middle pad will snap to Config[0].Default, it will also snap to 0 not to 50% as it does
    in the other example sketches! (It's not broken it's just configured differently!)

    Play with the presets first, and find out what they do.
  */

  // Initialization (It should not affect anything as Config[0].Default is 0 in this sketch, but it's required just in case you play with the default value...)
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
  Serial.println ();
  Serial.println ();

  // It starts at position 0 but only displays it if there's an event. There's no event for position 0, but this should print it regardless at start so it's not missing. (Just a nice touch...)
  Serial.print (F("CPos: "));
  Serial.print (TB.GetPositionFloat()); // Displaying current position in percentage.
  Serial.print (F("   CPos-Log: "));
  Serial.print (Logarithmic(TB.GetPositionFloat())); // Displays the current position's Logarithmic equivalent in percentage.
  Serial.print (F("   CPos-InverseLog: "));
  Serial.print (InverseLogarithmic(TB.GetPositionFloat())); // Displays the current position's InverseLogarithmic equivalent in percentage.
  Serial.print (F("   CPos-S-Curve: "));
  Serial.print (SCurve(TB.GetPositionFloat())); // Displays the current position's S-Curve equivalent in percentage.
  Serial.println ();
}

void loop()
{
  PreviousTarget = TB.GetTargetInt();
  // Read pads and feed it to Touch Bar lib.
  TB.Update (TInA.ReadState(), TInB.ReadState(), TInC.ReadState());

  // Get position.
  if (TB.Event() == true)
  {
    Serial.print (F("CPos: "));
    Serial.print (TB.GetPositionFloat()); // Displaying current position in percentage.
    Serial.print (F("   CPos-Log: "));
    Serial.print (Logarithmic(TB.GetPositionFloat())); // Displays the current position's Logarithmic equivalent in percentage.
    Serial.print (F("   CPos-InverseLog: "));
    Serial.print (InverseLogarithmic(TB.GetPositionFloat())); // Displays the current position's InverseLogarithmic equivalent in percentage.
    Serial.print (F("   CPos-S-Curve: "));
    Serial.print (SCurve(TB.GetPositionFloat())); // Displays the current position's S-Curve equivalent in percentage.
    Serial.println ();
    // In case you make a graph: Open serial monitor, clear the output, reset the arduino, and hit the top pad and it will print a list you can insert into an excel sheet.
  }
}
