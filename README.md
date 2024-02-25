# PALDump
Combinatorial Programmable Array Logic dumper for Arduino

Principle of operation:
 1. Detection of tri-state outputs using the dedicated probe pin.
 2. Analysis of outputs. It reads each output pin and counts occurunces
    of each state for all 65K/1M of input combinations, detecting inputs,
    outputs, I/O or buffered outputs, and builds an internal table of active
    outputs as the boolean function outputs with addition of Output Enable
    functions per each buffered output.
 3. Analysis of inputs. It tests each pin that could be used as a logic input
    and detects if any change at this input doesn't affect output functions.
    Then it build a table of usable inputs.
 4. It generates the optimized truth table (with excluded unused inputs)
    in Berkeley format and transfers it via Serial port (USB/UART1) at
    115200 bps.

Requirements:
 1. An Arduino development board. Mega 2560 is recommended.
 2. 2x complete I/O ports (7..0), which aren't shared with Serial.
 3. 1x I/O port having at least pins 5..0 available.
 4. 1x I/O pin for probe
 5. A breadbord with a set of jumper wires
 6. 25x Dupont/compatible Male-Male wires
 7. 8x thru-hole resistors of 220-270 Ohms
 8. 8x thru-hole resistors or one-to-many resistor array of 1-1.5 KOhm
 9. A terminal program which can save serial communication log to a file
    (Arduino IDE 2.0+ doesn't allow exporting or copying of the serial
    monitor output, so PuTTY does the work perfectly.)
10. Espresso Logic Minimizer installed. (Free)
11. A CPLD development software such as Atmel WinCUPL for generating the
    .jed files

Usage:
 1. Connect the PLA to the Arduino as shown on the schematic (See PALDump.h)
 2. Set the constant PALDump::PALType globalPALType to PAL16L8 or PAL20L8
 3. Reupload the project
 4. Connect with the Arduino serial port using a terminal program and save
    the full log to a text file.
 5. Clean the header if needed and feed the file to Espresso using the
    following command:

    espresso -Dexact -Dmany -o eqntott inputfile.pld > outputfile.pld

 6. Analyze and reformat the output in order to use it for designing
    your own clone of PLA using CPLD design software of your choice.

Precautions:
*  Analysis of PAL16xx usually takes up to 20 seconds while analysis
   PAL20xx may take 16 times more, but in exchange it stripes the size
   of the complete truth table by 2^n times where n is non-significant
   inputs.
*  It's recommended to read several copies of truth table and compare
   them using fc or another file compare utility to detect errors.
   Automatic optional error-correction will be added later, but it
   will take more time.
*  This tool isn't designed as a blind push-button tool, however most of
   hardwork is performed in fully automatic mode. The output still needs
   evaluation with some degree of scepticism and common sense.
   

