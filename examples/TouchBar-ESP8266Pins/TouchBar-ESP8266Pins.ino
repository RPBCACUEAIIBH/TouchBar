/*
The TouchBar library is an engine designed to take an input, of 3 bits (first 3 bits of a byte such as the output of my TouchLib library or the Adafruit_MPR121 library) and interpret it as a touch bar.


Hardware requirements:
- ESP8266 (I'm using ESP12F a "Generic ESP8266 Module") Note that the TouchLib and TouchBar libraries are frequency dependent, so if your board works on other then 80MHz you may require different settings...
- Touch bar hooked up to 3 digital inputs, and one more to a function pad. Each with an external 330K pullup resistor.


Libraries requirements:
- ESP8266 software (in Tools/Board/Board Manager...)
- TouchLib (Available among my github repositories: https://github.com/RPBCACUEAIIBH/TouchLib)
- TouchBar (This one...)
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
#include <EEPROM.h> // If you want to use SaveTouchBarConfig () and LoadTouchBarConfig () you must also include EEPROM.h and initialize EEPROM library in setup () calling EEPROM.begin () as the ESP does not actually have EEPROM instead it uses Flash.

// TouchLib objects
DigitalTouch TInA(13);
DigitalTouch TInB(12);
DigitalTouch TInC(14);
DigitalTouch TInF(16);
// This works with digital pins, there's only one Analog pin on the 12F anyway... :)

// TouchBar objects
// The Common object applies to every TouchBar object. These are tuning options, and should be left untouched unless your ESP8266 runs on other then 80MHz, or your program is so long, it's getting slow...
TouchBarCommon Common = {800, 250}; // unsigned int TapTimeout, byte TwitchSuppressionDelay 
/*
  Note:
  - TapTimeout - may not be obvious, if you rest your finger on the touch bar, but you change your mind and don't wanna ajust it, any you're only touching 1 of the pads, it may interpret it as a tap,
    which could lead to trouble when snap functionality is enabled. TapTimeout is designed to prevent that. (It works, but not exactly foolproof, if you go crazy on the touchbar it may snap every
    once in a while. For critical application where an accidental tap may lead to serious consequences, I'd recommend only enabling snap functionality in the special mode when the function pad is held!)
  - TwitchSuppressionDelay - Selective debouncing, which suppresses oscillation on a pad, but passes change of state on any other pad then the last active.
  - Both values refer to cycles of execution, not milliseconds or microseconds, thus frequency and long loops in your program may affect it!
  - If your ESP runs faster then 40MHz you need to reduce resistor value even lower then 330K cause TwitchSuppressionDelay only goes up to 255.
*/

// Any of the config objects are modes of operation for the TouchBar objects, and can be used by one or more TouchBar objects at once.
TouchBarConfig Config[2];

TouchBar TB (&Common, &Config[0]); // It takes: TouchBarCommon *CommonPtr, TouchBarConfig *ConfigPtr

// Variables
byte FunctionPadState = 0;
unsigned int PreviousTarget;

void setup()
{
  Serial.begin (115200);
  Serial.println (); // Thes ESP print some information at the beginning which may appear gibberish.
  Serial.println (); // This 2 newline separates it form the messages the sketch sends...

  /* TouchLib */
  /* ESP8266 settings (ESP12F generic module) */
  TInA.SetCalibLimit (20);
  TInA.SetThreshold (8);
  TInB.SetCalibLimit (20);
  TInB.SetThreshold (8);
  TInC.SetCalibLimit (20);
  TInC.SetThreshold (8);
  TInF.SetCalibLimit (20);
  TInF.SetThreshold (25);
  
  /*
  Debug code (use it to tune your pads if needed. This can be touchy, so clean up flux residue, before tuning as it may infuence the readings, create crosstalk between pins, etc.)
  You will need to use it if your ESP runs on other then 80MHz, you didn't clean flux residue, you used other then 330K resistors or if you have more interference then I have...
  It was originally designed for 16MHz AVRs and it's really sensitive due to the ESP's high speed...
  
  - TInX.SetCalibLimit () should be somewhat a higher value then what it reads when not touched.
  - Stat will be high when Untouched value + TInX.SetThreshold () value is exceeded.
  TInA.Calibrate() will wait till the pad is released, read actual untouched value, and calculat when the state should turn HIGH.
    Once the stat is HIGH it will stay high until the readings retunr to untouched value to avoid oscillation.
  */
  /*
  while(1)
  {
    byte X = TInA.Read ();
    if (X < 10)
      Serial.print ("TInA:   ");
    else if (X < 100)
      Serial.print ("TInA:  ");
    else
      Serial.print ("TInA: ");
    Serial.print (X);
    X = TInB.Read ();
    if (X < 10)
      Serial.print ("   TInB:   ");
    else if (X < 100)
      Serial.print ("   TInB:  ");
    else
      Serial.print ("   TInB: ");
    Serial.print (X);
    X = TInC.Read ();
    if (X < 10)
      Serial.print ("   TInC:   ");
    else if (X < 100)
      Serial.print ("   TInC:  ");
    else
      Serial.print ("   TInC: ");
    Serial.print (X);
    X = TInF.Read ();
    if (X < 10)
      Serial.print ("   TInF:   ");
    else if (X < 100)
      Serial.print ("   TInF:  ");
    else
      Serial.print ("   TInF: ");
    Serial.print (X);
    Serial.println ();
    yield();
  }
  */

  while (TInA.Calibrate())
  {
    Serial.println (F("Calibration for Touch Input A failed! Retrying..."));
    yield ();
  }
  while (TInB.Calibrate())
  {
    Serial.println (F("Calibration for Touch Input B failed! Retrying..."));
    yield ();
  }
  while (TInC.Calibrate())
  {
    Serial.println (F("Calibration for Touch Input C failed! Retrying..."));
    yield ();
  }
  while (TInF.Calibrate())
  {
    Serial.println (F("Calibration for Touch Function input failed! Retrying..."));
    yield ();
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
  /*
  EEPROM.begin (1024); // ESP8266 doesn't actually have an EEPROM, in this case it allocates 1024 bytes of flash memory where settings will be saved.
  SaveTouchBarConfig (&Common, Config, sizeof(Config)/sizeof(Config[0]), 0); // Can be left uncommented, it uses EEPROM.update()
  LoadTouchBarConfig (&Common, Config, sizeof(Config)/sizeof(Config[0]), 0);
  */

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

void loop()
{
  /* Function Pad */
  // Touch readings on ESP are really sensitive so I recommend checking double-triple-quadruple checking like so...
  FunctionPadState <<= 1;
  bitWrite (FunctionPadState, 0, TInF.ReadState());
  if (FunctionPadState == 0x0F)
  {
    Serial.println (F("The Function pad got touched!"));
    // This should temporarily enable ramp mode, as long as you hold the function pad. (This presents 2 new features. SetTaget() and Restore())
    TB.Reconfigure (&Config[1]);
  }
  if (FunctionPadState == 0xF0)
  {
    Serial.println (F("The Function pad got released!"));
    // Restore previous settings from EEPROM without reseting current position and target.
    TB.Reconfigure (&Config[0]);
  }

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
