# PALDump
Combinatorial Programmable Array Logic dumper for Arduino

Features:
 1. Detection of tri-state outputs using the dedicated probe pin.
 2. Analysis of outputs. It reads each output pin and counts occurunces of each state for all 65K/1M of input combinations, detects inputs, outputs, I/O or buffered outputs and builds a list of output functions per physical output and Output Enable.
 3. Analysis of inputs. It tests each input or I/O pin and detects if any change at this input doesn't affect output functions. Then it builds a table of usable inputs.
 4. It generates the optimized truth table (with excluded unused inputs) in Berkeley format and transfers it via Serial port. (USB/UART1 by default)

Requirements:
 1. An Arduino development board. MEGA2560 is recommended. Uno and other ATMEGA328 based boards aren't supported!
 2. 3x complete I/O ports (7..0), which aren't shared with Serial.
 3. 1x additional I/O pin for probe
 4. A breadboard with a set of jumper wires
 5. 25x Dupont/compatible Male-Male wires
 6. 8x thru-hole resistors of 220..270 Ohms
 7. 8x thru-hole resistors or one-to-many resistor array of 1..1.5 KOhm
 8. A terminal program which can save serial communication log to a file. (Arduino IDE 2.0+ doesn't allow exporting or copying of the Serial Monitor output, so PuTTY is recommended)
 9. Installed Espresso Logic Minimizer. (Free)
10. A CPLD development software such as Atmel WinCUPL for generating the .jed files

Usage:
 1. Connect the PLA chip to the Arduino as shown on the schematic (See PALDump.h)
 2. Set the constant PALDump::PALType globalPALType to PAL16L8 or PAL20L8
 3. Recompile and upload the project into Aruino board.
 4. Connect with the Arduino serial port using a terminal program and save the full log to a text file.
 5. Clean the header if needed and feed the file to Espresso using the following command:

    espresso -Dexact -Dmany -o eqntott inputfile.pld > outputfile.pld

 6. Analyze and reformat the output in order to use it for designing your own clone of PLA using CPLD design software of your choice. (Invert boolean functions for outputs, keep for .OE, remove input terms for buffered outputs, which are duplicated in .OE equations, etc)

Precautions:
*  Serial communication at 115200 bps is slow for transferring thousands of text lines. It can take up to 200 seconds for transferring 65K entries and up to 1 hour (!) for 1M entries, so program does it best in advance to exclude unneeded inputs and outputs from the truth table which halfs its size with each excluded input . Analysis of PAL16xx usually takes up to 20 seconds while full analysis of PAL20xx may take 16 times more, but eventually it saves up to 10 times more time. Using faster serial communication is also adviced.
*  It's recommended to read several copies of truth table and compare them using fc or another file compare utility to detect errors. Automatic optional error-correction will be added later, but it's expected to consume more time.
*  This tool isn't designed as a blind push-button solution, although most of hardwork is performed in fully automatic mode along with Espresso. The output still needs evaluation with a certain degree of scepticism and common sense.
   

