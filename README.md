The TouchBar library is an engine designed to take an input, of 3 bits (first 3 bits of a byte such as the output of the Adafruit_MPR121 library, or a port register) and interpret it as a touch bar.


Features:
- It senses directon whether you slide your finger over 1-2 pads(light touch scenario), or 2-3 pads at once (hard touch scenario) so it has some tolerance.
- It senses quick tap on all 3 pads, so the same pads can be used as "buttons" except you loose the ability to hold them down(If it's held, it expects you to drag your finger on the bar.),
  they can only be tapped as buttons.
- It can operate with a limited range, or roll over infinitely
- With limited range, you get built in springback, snap, ramp and flip features.
  - If springback is enabled, it resets to default position when you release the touchbar. It will supress snap feature, but works with ramp also enabled.
  - If snap feature is enabled, tapping the pads will snap the position to minimim / default / maximim positions. This also works fine with the ramp feature.
  - When ramp is enabled, you're adjusting a target position, and the actual position will slowly move to the target position. In this case, you also have an adjustment resolution, and a ramp resolution,
    so you can make coarese adjustment on the touch bar, but the automatic adjustment will have a fine steps at and even speed.
  - Flip feature just flips pads A and C (Reverses scroll direction, and swaps Top and Bottom snap pads as well, just like you would orient it upside down.)
- Settings requre 12 bytes of EEPROM.
- It has twitch suppression which is a form of advanced parallel debouncing that suppresses fast oscillation of a pad, but immediately passes trough change on any other pad then the last one. (This is done to prevent false tap reading and thus snapping when barely touching the edge of a pad, such as placing/removing your finger from the touchbar, or swiping very lightly. False taps ware fairly common without this feature. Simple debouncing as it is commonly done for buttons would limit swipe speed...)


Hardware requirements:
- Arduino running at 16MHz (Any type should do. 16MHz is important cause delays are based on cycles of execution not ms / us, 8MHz will work but require different settings. Mine is pro-mini 5V, 16MHz version.)
- 3.3V regulator if your arduino doesn't have 3.3V power on it, and level shifter. (Ignore this if your arduino somehow runs at 3.3V, 16MHz)
- MPR 121 touch module hooked up to i2c via the level shifter(cause it's a 3.3V device)
- Touch bar hooked up to 0,1 and 2 touch touch inputs of the MPR121 module according to the provided documentation.


Software requirements:
- Arduino IDE
- KiCAD
- Adafruit_MPR121 library (or similar... not included, you need to install it separately!)
- Wire library (required by Adafruit_MPR121, Should be included with your IDE)
- TouchBar library (This one...)
- EEPROM library (Required by TouchBar, Should be included with your IDE)


Skill requirements:
- You need to have basic arduino skills. (Understanding how to interface 5V and 3.3V modules, hooking up and testing an i2c device with level shifter, soldering, understanding arduino code, installing and using libraries, etc. This is a library of source code and CAD files with an example sketch not a tutorial, so I won't explain everything here.)
- You either need to make a PCB or order one! (The touch bar itself is basically a footprint you have to print on a PCB, it's a custom design, you can't really buy it.)


Instructions:
1. Add library to your arduino IDE 
 - Open terminal in your Arduino/libraries directory, and run "git clone https://github.com/RPBCACUEAIIBH/TouchBar" and restart the IDE.
 - Download as .zip package, go to the Sketch/Include Library/Add .ZIP Library... in you Arduino IDE, select it, and click ok.
2. Open the included example sketch to see how to use it.
3. It includes a KiCAD library I made with few symbols and several footprints so you can make a proper touch bar on PCB.
