/*
The TouchBar library is an engine designed to take an input, of 3 bits (first 3 bits of a byte such as the output of my TouchLib library or the Adafruit_MPR121 library.) and interpret it as a touch bar.


Hardware requirements:
- Arduino running at 16MHz (Any type should do. 16MHz is important cause delays are based on cycles of execution not ms / us, 8MHz will work but require different settings. Mine is pro-mini 5V, 16MHz version.)
- Touch bar hooked up to 4 digital inputs, each with an external 1M pullup resistor.


Libraries requirements:
- TouchLib (Available among my github repositories: https://github.com/RPBCACUEAIIBH/TouchLib)
- TouchBar (This one...)
- EEPROM (Required by TouchBar, Should be included with your IDE)


Skill requirements:
- You need to have basic arduino skills. (Understanding how to interface 5V and 3.3V modules, hooking up and testing an i2c device with level shifter, soldering, understanding arduino code, installing and using libraries, etc. This is a library of source code and CAD files with an example sketch not a tutorial, so I won't explain everything here.)
- You either need to make a PCB or order one! (The touch bar itself is basically a footprint you have to print on a PCB, it's a custom design, you can't really buy it.)


If you find this useful, please consider donationg: http://osrc.rip/Support.html
*/

#include <TouchLib.h>
#include <TouchBar.h>

DigitalTouch TInA(A0);
DigitalTouch TInB(A1);
DigitalTouch TInC(A2);
DigitalTouch TInF(A3);
boolean PreviousFunctionState = false;

TouchBar TB (0);
unsigned int PreviousTarget;

void setup()
{
  Serial.begin (115200);
  /* TouchLib */
  // These are set for 16MHz arduinos, you may need to tweak them for different frequency.
  // TInA.SetCalibLimit (); // Calibration will fail if this value is exceeded. It takes: byte NewLimit; Digital Default: 7;
  // TInA.SetThreshold (); // This determines the sensitivity; It takes: byte NewThreshold; Default: 3; Valid: >= 3;
  while (TInA.Calibrate())
    Serial.println (F("Calibration for Touch Input A failed! Retrying..."));
  while (TInB.Calibrate())
    Serial.println (F("Calibration for Touch Input B failed! Retrying..."));
  while (TInC.Calibrate())
    Serial.println (F("Calibration for Touch Input C failed! Retrying..."));
  while (TInF.Calibrate())
    Serial.println (F("Calibration for Touch Function input failed! Retrying..."));

  /* TouchBar */
  // Write presets to EEPROM (Optional, only required once.)
  TB.SetDefault (5000, true); // It takes: unsigned int NewDefault, boolean SaveToEEPROM(optional)
  TB.SetLimit (10000, true); //It takes: unsigned int NewLimit, boolean SaveToEEPROM(optional)
  TB.SetResolution (100, true); // It takes: byte NewResolution, boolean SaveToEEPROM(optional)
  TB.SetRampDelay (100, true); // It takes: byte NewRampDelay, boolean SaveToEEPROM(optional)
  TB.SetRampResolution (25, true); // It takes: byte NewResolution, boolean SaveToEEPROM(optional)
  TB.SetTapTimeout (300, true); // It takes: unsigned int NewTapTimeout, boolean SaveToEEPROM(optional); ~300 value should be good(with my TouchLib library), if your program is long or your arduino runs slower then 16MHz, you may need to reduce it. Again it's in Cycles, not in ms or us, so it changes with execution speed and long loops will affect it!
  TB.SetTSDelay (60, true); // Takes byte TSDealy, boolean SaveToEEPROM(optional); 50 works fine with 16MHz arduino(and with my TouchLib library). The higher the number the better, however if it's too high it may miss a very fast tap. (Debounces oscillation of the same pad, but immediately passes trough change on any other pad then the previous one to allow for fast swipe. Without this sometimes it may falsely read a tap on a pad when placing or removing your finger on the touch bar.)
  // Use only one of the following 2 lines! It is an overloaded method, one of them should be commented!
  TB.SetFlags (false, true, false, false, true); // It takes: boolean SpringBackFlag, boolean SnapFlag, boolean RampFlag, boolean FlipFlag, boolean SaveToEEPROM(optional)
  //TB.SetFlags (true, false, true); // it takes: boolean RollOverFlag, boolean FlipFlag, boolean SaveToEEPROM(optional)
  /*
  Note:
  - You don't need to save to EEPROM every time. The EEPROM is for preserving settings when it's turned off.
  - Writing EEPROM is relatively slow, and it has a limited number of writes.
  - The TouchBar library is using EEPROM.update() rather then EEPROM.write() to avoid excessive writes, so it's safe to leave the above code in the sketch.
  - You can either have limited range, and fancy features, or unlimited range, and no fancy features...
  - Snap does nothing when springback is also enabled.
  - TapTimeout may not be obvious, but it is implemented to avoid sensing it as a tap if you rest your finger on the touch bar, but you change your mind and don't wanna ajust it.

  Play with the presets first, and find out what they do.
  */
  
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
}

void loop()
{
  /* ToucLib */
  boolean X = TInF.ReadState();
  if (X == HIGH && PreviousFunctionState == LOW)
  {
    Serial.println (F("The Function pad got touched!"));
    // This should temporarily enable ramp mode, as long as you hold the function pad. (This presents 2 new features. SetTaget() and Restore())
    TB.SetResolution (350); // This will temporarily set coarser resolution for target adjustment but will ramp with fine steps...
    TB.SetFlags (false, true, true, false); // This enables ramp function.
    TB.SetTarget(TB.GetPositionInt()); // Adjust target to current position to avoid ramping back to previous target immediately...
  }
  if (X == LOW && PreviousFunctionState == HIGH)
  {
    Serial.println (F("The Function pad got released!"));
    TB.Restore (); // Restore previous settings from EEPROM without reseting current position and target.
  }
  PreviousFunctionState = X;

  /* TouchBar */
  PreviousTarget = TB.GetTargetInt();
  // Read pads and feed it to Touch Bar lib.
  TB.Update (TInA.ReadState(), TInB.ReadState(), TInC.ReadState());

  // Get tap.
  if (TB.PadEvent() != 'Z')
  {
    Serial.print (F("Tapped the "));
    Serial.print (TB.PadEvent());
    Serial.println (F(" pad."));
  }

  // Get position.
  if (TB.Event() == true)
  {
    if (TB.GetTargetInt () != PreviousTarget)
    {
      Serial.print (F("Target set to "));
      Serial.print (TB.GetTargetFloat()); // Displaying target position in percentage.
      Serial.println (F("%"));
    }
    Serial.print (F("CPos: "));
    Serial.print (TB.GetPositionFloat()); // Displaying current position in percentage.
    Serial.println (F("%"));
  }
}
