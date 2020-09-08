/*
The TouchBar library is an engine designed to take an input, of 3 bits (first 3 bits of a byte such as the output of my TouchLib library or the Adafruit_MPR121 library.) and interpret it as a touch bar.


Hardware requirements:
- Arduino running at 16MHz (Any type should do. 16MHz is important cause delays are based on cycles of execution not ms / us, 8MHz will work but require different settings. Mine is pro-mini 5V, 16MHz version.)
- 3.3V regulator if your arduino doesn't have 3.3V power on it, and level shifter. (Ignore this if your arduino somehow runs at 3.3V, 16MHz)
- MPR 121 touch module hooked up to i2c via the level shifter(cause it's a 3.3V device)
- Touch bar hooked up to 0,1 and 2 touch touch inputs of the MPR121 module according to the provided documentation.


Libraries requirements:
- Adafruit_MPR121 (or similar... not included, you need to install it separately!)
- Wire (required by Adafruit_MPR121, Should be included with your IDE)
- TouchBar (This one...)
- EEPROM (Required by TouchBar, Should be included with your IDE)


Skill requirements:
- You need to have basic arduino skills. (Understanding how to interface 5V and 3.3V modules, hooking up and testing an i2c device with level shifter, soldering, understanding arduino code, installing and using libraries, etc. This is a library of source code and CAD files with an example sketch not a tutorial, so I won't explain everything here.)
- You either need to make a PCB or order one! (The touch bar itself is basically a footprint you have to print on a PCB, it's a custom design, you can't really buy it.)


If you find this useful, please consider donationg: http://osrc.rip/Support.html
*/

#include <Adafruit_MPR121.h>
#include <TouchBar.h>

// MPR121 Driver Object
Adafruit_MPR121 TouchModule = Adafruit_MPR121();
int CTouched = 0;
int PTouched;

// Touch Bar Object
TouchBar TB(0); // It takes an EEPROM address (Uses 12 bytes of EEPROM starting from this address, so in this case the first 12 bytes of EEPROM will be used.)
/* 
Note:
- You can have up to 4 touch bars with the same MPR121 module since a touch bar requires only 3 touch inputs, and it has 12.
- 2 or more instances can share the same 12 bytes of EEPROM provided that the same initial settings are fine for each.
*/

// Variables used for this example sketch.
unsigned int PreviousTarget;


void setup ()
{
  Serial.begin(115200); // <<< 9600 is slow... (...at least for me. :P)

  // First initialize the MPR121 library
  if (!TouchModule.begin(0x5A))
  {
    Serial.println(F("MPR121 not found!"));
    while (1);
  }

  // Write presets to EEPROM (Optional, only required once.)
  TB.SetDefault (5000, true); // It takes: unsigned int NewDefault, boolean SaveToEEPROM(optional)
  TB.SetLimit (10000, true); //It takes: unsigned int NewLimit, boolean SaveToEEPROM(optional)
  TB.SetResolution (100, true); // It takes: byte NewResolution, boolean SaveToEEPROM(optional)
  TB.SetRampDelay (10, true); // It takes: byte NewRampDelay, boolean SaveToEEPROM(optional)
  TB.SetRampResolution (25, true); // It takes: byte NewResolution, boolean SaveToEEPROM(optional)
  TB.SetTapTimeout (200, true); // It takes: unsigned int NewTapTimeout, boolean SaveToEEPROM(optional); ~200 value should be good, if your program is long or your arduino runs slower then 16MHz, you may need to reduce it. Again it's in Cycles, not in ms or us, so it changes with execution speed and long loops will affect it!
  TB.SetTSDelay (40, true); // Takes byte TSDealy, boolean SaveToEEPROM(optional); 20-45 works fine with 16MHz arduino. The higher the number the better, however if it's too high it may miss a very fast tap. (Debounces oscillation of the same pad, but immediately passes trough change on any other pad then the previous one to allow for fast swipe. Without this sometimes it may falsely read a tap on a pad when placing or removing your finger on the touch bar.)
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

void loop ()
{
  PreviousTarget = TB.GetTargetInt();
  // Read pads and feed it to Touch Bar lib.
  TB.Update (TouchModule.touched()); // Takes byte but only uses first 3 bits [0-2]
  /*
  // You can also do something like this giving it 3 boolean values instead of a byte, however it uses 30 bytes more program space... (Don't forget to comment the line above if you uncomment the following 2 lines!)
  unsigned int X = TouchModule.touched();
  TB.Update (bitRead(X, 0), bitRead(X, 1), bitRead(X, 2));
  */

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