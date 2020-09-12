/*
The TouchBar library is an engine designed to take an input, of 3 bits (first 3 bits of a byte such as the output of my TouchLib library or the Adafruit_MPR121 library.) and interpret it as a touch bar.


Hardware requirements:
- Arduino running at 16MHz (Any type should do. 16MHz is important cause delays are based on cycles of execution not ms / us, 8MHz will work but require different settings. Mine is pro-mini 5V, 16MHz version.)
- Touch bar hooked up to 3 digital inputs, each with an external 1M pullup resistor.


Libraries requirements:
- TouchLib (Available among my github repositories: https://github.com/RPBCACUEAIIBH/TouchLib)
- TouchBar (This one...)
- Delinearizer (Available among my github repositories: https://github.com/RPBCACUEAIIBH/Delinearizer)
- EEPROM (Required by TouchBar, Should be included with your IDE)


Skill requirements:
- You need to have basic arduino skills. (Understanding how to interface 5V and 3.3V modules, hooking up and testing an i2c device with level shifter, soldering, understanding arduino code, installing and using libraries, etc. This is a library of source code and CAD files with an example sketch not a tutorial, so I won't explain everything here.)
- You either need to make a PCB or order one! (The touch bar itself is basically a footprint you have to print on a PCB, it's a custom design, you can't really buy it.)


If you find this useful, please consider donationg: http://osrc.rip/Support.html
*/

#include <TouchLib.h>
#include <TouchBar.h>
#include <Delinearizer.h>

DigitalTouch TInA(A0);
DigitalTouch TInB(A1);
DigitalTouch TInC(A2);
boolean PreviousFunctionState = false;

TouchBar TB (0);
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
  // Write presets to EEPROM (Optional, only required once.)
  TB.SetDefault (0, true); // Set to 0 for this sketch. 
  TB.SetLimit (10000, true);
  TB.SetResolution (100, true);
  TB.SetRampDelay (100, true);
  TB.SetRampResolution (25, true);
  TB.SetTapTimeout (300, true);
  TB.SetTSDelay (60, true);
  TB.SetFlags (false, true, true, false, true); // Snap and Ramp is set to true so by hitting the top pad you get all 4 type of output from 0% to 100%. You can copy that into excel, and make a graph.
  
  // Second initialize the Touch Bar library (load presets form EEPROM)
  TB.Init (); // Init() loads settings from EEPROM, but you can set everthing directly if you want instead of calling Init() just as you would write presets to EEPROM with the above section, except without specifying the "SaveToEEPROM" value at the end. (It defaults to false.)

  // Report after init...
  Serial.print (F("Reporting Touch Bar status:"));
  Serial.print (F("\n  Default = "));
  Serial.print (TB.GetDefault());
  Serial.print (F("\n  Limit = "));
  Serial.print (TB.GetLimit ());
  Serial.print (F("\n  Resolution = "));
  Serial.print (TB.GetResolution ());
  Serial.print (F("\n  RampDelay = "));
  Serial.print (TB.GetRampDelay ());
  Serial.print (F("\n  RampResolution = "));
  Serial.print (TB.GetRampResolution ());
  Serial.print (F("\n  TapTimeout = "));
  Serial.print (TB.GetTapTimeout ());
  Serial.print (F("\n  TSDelay = "));
  Serial.print (TB.GetTSDelay ());
  Serial.print (F("\n  RollOverFlag = "));
  Serial.print (TB.GetRollOverFlag ());
  Serial.print (F("\n  SpringBackFlag = "));
  Serial.print (TB.GetSpringBackFlag ());
  Serial.print (F("\n  SnapFlag = "));
  Serial.print (TB.GetSnapFlag ());
  Serial.print (F("\n  RampFlag = "));
  Serial.print (TB.GetRampFlag ());
  Serial.print (F("\n  FlipFlag = "));
  Serial.print (TB.GetFlipFlag ());
  Serial.println ();
  Serial.println ();
  
  Serial.println(F("Initialization done!"));

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
