#ifndef paldump_h
#define paldump_h

#include "Arduino.h"
#include <stdint.h>

/*
 * Programmable Logic Array reading class. Uses three full GPIO ports of a larger Arduino board.
 * 
 *                  PAL16L8                     PAL20L8
 *                  ______                      ______   
 *           In 1 -|      |- Vcc         In 0 -|      |- Vcc
 *           In 2 -|      |- Out 8       In 1 -|      |- In 12
 *           In 3 -|      |- I/O 7       In 2 -|      |- Out 8
 *           In 4 -|      |- I/O 6       In 3 -|      |- I/O 7
 *           In 5 -|      |- I/O 5       In 4 -|      |- I/O 6
 *           In 6 -|      |- I/O 4       In 5 -|      |- I/O 5
 *           In 7 -|      |- I/O 3       In 6 -|      |- I/O 4
 *           In 8 -|      |- I/O 2       In 7 -|      |- I/O 3
 *           In 9 -|      |- Out 1       In 8 -|      |- I/O 2
 *            GND -|      |- In 10       In 9 -|      |- Out 1
 *                 ''''''''             In 10 -|      |- In 11
 *                                        GND -|      |- In 13
 *                                             ''''''''
 *           _____________PORT3_____________     _____________PORT2_____________     _____________PORT1_____________
 *           [7] [6] [5] [4] [3] [2] [1] [0]     [7] [6] [5] [4] [3] [2] [1] [0]     [7] [6] [5] [4] [3] [2] [1] [0]
 * PAL16L8:  O8  IO7 IO6 IO5 IO4 IO3 IO2 O1                              I10 I9      I8  I7  I6  I5  I4  I3  I2  I1
 * PAL20L8:  O8  IO7 IO6 IO5 IO4 IO3 IO2 O1      PRB     I13 I12 I11 I10 I9  I8      I7  I6  I5  I4  I3  I2  I1  I0
 *                                                ^ 
 *                                                \---- possible adoption. PORTG2 is used in the original project.
 *
 *             270R                       1K array
 * Out 8 -----^^^^^^-----[PORT3 PIN 7]-----^^^^^^-----+
 * I/O 7 -----^^^^^^-----[PORT3 PIN 7]-----^^^^^^-----+
 * I/O 6 -----^^^^^^-----[PORT3 PIN 7]-----^^^^^^-----+
 * I/O 5 -----^^^^^^-----[PORT3 PIN 7]-----^^^^^^-----+
 * I/O 4 -----^^^^^^-----[PORT3 PIN 7]-----^^^^^^-----+
 * I/O 3 -----^^^^^^-----[PORT3 PIN 7]-----^^^^^^-----+
 * I/O 2 -----^^^^^^-----[PORT3 PIN 7]-----^^^^^^-----+
 * Out 1 -----^^^^^^-----[PORT3 PIN 7]-----^^^^^^-----+
 *                                                    |
 *                                                    |
 *                                               [Probe pin]
 */

typedef struct {
  uint8_t low;
  uint8_t high;
} uint16;

typedef struct {
  uint8_t byte1;
  uint8_t byte2;
  uint8_t byte3;
  uint8_t byte4;
} uint32;

#define NOP __asm__ __volatile__("nop")
#define DIR_PAL16L8 0b01111110
#define LO8(x) (*(volatile uint16 *)(&x)).low
#define HI8(x) (*(volatile uint16 *)(&x)).high
#define BYTE1(x) (*(volatile uint32 *)(&x)).byte1
#define BYTE2(x) (*(volatile uint32 *)(&x)).byte2
#define BYTE3(x) (*(volatile uint32 *)(&x)).byte3
#define BYTE4(x) (*(volatile uint32 *)(&x)).byte4
#define DDR(x) *(&x-1)
#define PIN(x) *(&x-2)

// Already defined in the main project file
#define PROBEPORT PORTG
#define PROBEPIN 2


class PALDump {

  public:
    typedef union {
      uint16_t port;
      struct {
        unsigned state1 : 2;
        unsigned state2 : 2;
        unsigned state3 : 2;
        unsigned state4 : 2;
        unsigned state5 : 2;
        unsigned state6 : 2;
        unsigned state7 : 2;
        unsigned state8 : 2;
      };
      struct {
        unsigned output1 : 1;
        unsigned oe1     : 1;
        unsigned output2 : 1;
        unsigned oe2     : 1;
        unsigned output3 : 1;
        unsigned oe3     : 1;
        unsigned output4 : 1;
        unsigned oe4     : 1;
        unsigned output5 : 1;
        unsigned oe5     : 1;
        unsigned output6 : 1;
        unsigned oe6     : 1;
        unsigned output7 : 1;
        unsigned oe7     : 1;
        unsigned output8 : 1;
        unsigned oe8     : 1;
      };
    } PALWord;
    enum palPinType:uint8_t {Output = 0, Buffered = 1, Input = 2, Unknown = 3};
    enum triState:uint8_t {Low = 0, High = 1, HiZ = 3};
    enum PALType:uint8_t {PAL16L8 = 0, PAL20L8 = 1};

    PALDump(PALType type, volatile uint8_t *port1, volatile uint8_t *port2, volatile uint8_t *port3, uint8_t maxTries = 1);
/*
    void getCombinatorialOutputAsText(uint32_t combination, char *buffer);
    void getCombinatorialOutputAsText(char *buffer);
    uint16_t getCombinatorialOutput(uint32_t combination);
    uint16_t getCombinatorialOutput();
    void setCombination(uint32_t combination);
*/
    char readPALPin(uint32_t combination, uint8_t bit);
    bool analyzePAL();
    bool analyzeInputs();
    bool analyzeOutputs();
    bool inputIsSignificant(uint8_t in);
    palPinType *getPALConfig();
    PALWord getPort(uint32_t combination);
    uint32_t getPALInputMask();
    uint8_t getActiveOutputs();
    uint8_t getActiveInputs();
    uint8_t getMaxInputs();
    uint32_t shortToLongCombination(uint32_t combination);
    void assumeSimple14i8o();
    const char *pinType(PALDump::palPinType t);
  private:
    volatile uint8_t *HWPort1, *HWPort2, *HWPort3;
    PALType currentPALType;
    uint8_t currentPalPin; // Then pin being tested/read
    uint8_t posMask; // Mask for the PAL pin being read
    uint8_t negMask; // Copy of the above with inverted polarity
    palPinType palPinConfig[8]; // Array keeping the set of pal pin types
    palPinType palPinConfigCopy[8]; // Copy for export
    uint32_t palInputMask;
    uint8_t IOMask;
    uint32_t currentCombination;
    uint8_t maxRedundantReads;
    uint8_t activeInputs;
    uint8_t activeOutputs;
    uint32_t maxCombinations;
    uint8_t maxInputs;
    
    uint32_t sequentialCombination();
    palPinType convertSumToPinType(uint32_t sum);
};

#endif
