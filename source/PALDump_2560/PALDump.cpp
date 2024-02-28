#include "PALDump.h"

PALDump::PALDump(PALType type, volatile uint8_t *port1,volatile  uint8_t *port2, volatile uint8_t *port3, uint8_t maxTries) {
  if(maxTries > 4) maxTries = 4;
  if(maxTries < 1) maxTries = 1;
  maxRedundantReads = maxTries;
  HWPort1 = port1;
  HWPort2 = port2;
  HWPort3 = port3;
  currentPALType = type;
  currentCombination = 0;
  DDR(*HWPort1) = 255; // Output
  DDR(*HWPort2) = 255; // Output
  DDR(*HWPort3) = 0; // Input
  for(uint8_t pin = 0; pin<8; pin++) {
    palPinConfig[pin] = Unknown;
  }
  palInputMask = 0;
  activeInputs = 0;
  activeOutputs = 0;
  IOMask = DIR_PAL16L8;
  switch(currentPALType) {
    case PAL16L8:
      maxCombinations = 65536;
      maxInputs = 16;
      break;
    case PAL20L8:
      maxCombinations = 1048576;
      maxInputs = 20;
      break;
  }
}
/*
void PALDump::getCombinatorialOutputAsText(uint32_t combination, char *buffer) {
  
}
void PALDump::getCombinatorialOutputAsText(char *buffer) {
  
}

uint16_t PALDump::getCombinatorialOutput(uint32_t combination) {
  switch(currentPALType) {
    case PAL16L8:
      break;
    case PAL20L8:
      break;
  }
}
*/
char PALDump::readPALPin(uint32_t combination, uint8_t bit) {
  char res = '-';
  posMask = 1<<(bit-1);
  negMask = posMask ^ 255;
  switch(currentPALType) {
    case PAL16L8:
      *HWPort1 = BYTE1(combination);
      *HWPort2 = BYTE2(combination) & 0b00000011;
      *HWPort3 = BYTE2(combination)>>1;
      break;
    case PAL20L8:
      uint16_t ccopy;
      LO8(ccopy) = BYTE2(combination);
      HI8(ccopy) = BYTE3(combination);
      ccopy <<= 3;
      *HWPort1 = BYTE1(combination);
      *HWPort2 = BYTE2(combination);
      *HWPort3 = HI8(ccopy);
      break;
  }
  *HWPort3 &= IOMask;
  *HWPort3 &= negMask;
  DDR(*HWPort3) = IOMask & negMask;
  DDR(PROBEPORT) |= 1<<PROBEPIN;
  PROBEPORT |= 1<<PROBEPIN;
  if((PIN(*HWPort3)&posMask) == 0) res = '0';
  PROBEPORT &= (1<<PROBEPIN)^255;
  if((PIN(*HWPort3)&posMask) != 0) res = '1';
  DDR(PROBEPORT) &= (1<<PROBEPIN) ^ 255;
  DDR(*HWPort3) = 0;
  *HWPort3 = 0;
  return res;
}
/*
uint16_t PALDump::getCombinatorialOutput() {
  return getCombinatorialOutput(sequentialCombination());
}
void PALDump::setCombination(uint32_t combination) {
  currentCombination = combination;
}
uint32_t PALDump::sequentialCombination() {
  uint32_t o = currentCombination++;
  currentCombination &= (maxCombinations-1);
}
*/
void PALDump::assumeSimple14i8o() {
Serial.println("# Assumed 14 In / 8 Out configuration, no OE");
  activeOutputs = 8;
  activeInputs = 14;
  for(uint8_t i = 0; i<8; i++) {
    palPinConfig[i] = Output;
  }
  palInputMask = 0x00003fff;
  
}

bool PALDump::analyzePAL() {
  Serial.println("# Please wait. Analyzing PAL...");
  return analyzeInputs() && analyzeOutputs();
}
bool PALDump::analyzeOutputs() {
  Serial.println("# Analyzing outputs...");
  uint32_t a, b, c, a2, b2, c2;
  uint8_t success;
  a = 0; b = 0; c = 0; a2 = 0; b2 = 0; c2 = 0; success = 0;
  char pinState;
  for(uint8_t curPin = 8; curPin>0; curPin--) {
Serial.print("# Output "); Serial.print(curPin, DEC); Serial.print(" ");
    for(uint8_t t = 0; t<6; t++) {
      for(uint32_t comb = 0; comb<maxCombinations; comb++) {
        pinState = readPALPin(comb, curPin);
        switch(pinState) {
          case '-':
            a++;
            break;
          case '0':
            b++;
            break;
          case '1':
            c++;
            break;
        }
      }
      if(t>0) {
        if((a==a2)&&(b==b2)&&(c==c2)) success++;
        else success = 0;
      }
      else success++;
      a2 = a;
      b2 = b;
      c2 = c;
      a = 0; b = 0; c = 0;
      if(success>=maxRedundantReads) break;
    }
    if(success>=maxRedundantReads) {
      palPinConfig[curPin-1] = convertSumToPinType(a2);
      Serial.println(pinType(convertSumToPinType(a2)));
      switch(palPinConfig[curPin-1]) {
        case Output:
          activeOutputs++;
          break;
        case Buffered:
          activeOutputs += 2;
          break;
      }
    }
    else {
      Serial.print("Successful reads: "); Serial.println(success, DEC);
      return false;
    }
    success = 0;
  }
  return true;
}
bool PALDump::inputIsSignificant(uint8_t in) {
  uint32_t c, c2, pm, nm;
  uint16_t outs[2];
  uint8_t bit0, bitn;
  pm = (uint32_t)1<<in;
  nm = (pm ^ 0xffffffff) & 0xfffffffe;
//  Serial.print(" /"); Serial.print(pm, BIN); Serial.print("/"); Serial.print(nm, BIN); Serial.print("/");
  for(c = 0; c<maxCombinations; c++) {
      bit0 = c&1;
      if(in>0) {
        if((c&pm) != 0) bitn = 1; else bitn = 0;
        c2 = c & nm;
        c2 |= bitn;
        if(bit0 == 1) c2 |= pm;
      }
      else {
        c2 = c;
      }
      outs[bit0] = 0;
    outs[bit0] = getPort(c2).port;
    if(bit0 == 1) {
      if(outs[0] != outs[1]) {
//        Serial.print(" ("); Serial.print(c, BIN); Serial.print(","); Serial.print(c2, BIN); Serial.print(") ");
//        Serial.print(" ("); Serial.print(c2, DEC); Serial.print(") ");
        return true;
      }
    }
  }
  return false;
}
uint32_t PALDump::getPALInputMask() {
  if(palInputMask == 0) {
      analyzeInputs();
  }
  return palInputMask;
}
bool PALDump::analyzeInputs() {
  Serial.println("# Analyzing inputs...");
  palInputMask = 0;
  uint32_t checker = 1;
  activeInputs = 0;
  for(uint8_t i = 0; i < maxInputs; i++) {
    Serial.print("# Input "); Serial.print(i+1, DEC);
    if(inputIsSignificant(i)) {
      Serial.println(" OK");
      palInputMask |= checker;
      activeInputs++;
    }
    else {
      Serial.println(" doesn't matter");
    }
    checker = (uint32_t)checker << 1;
  }
/* The next line is logical and kept for further development but currently makes it worse */
/*  IOMask &= (((uint8_t)palInputMask>>(maxInputs-7)) | 0b10000001); */
  if(palInputMask != 0) return true; else return false;
}
uint8_t PALDump::getActiveOutputs() {
  return activeOutputs;
}
uint8_t PALDump::getActiveInputs() {
  return activeInputs;
}
uint8_t PALDump::getMaxInputs() {
  return maxInputs;
}
uint32_t PALDump::shortToLongCombination(uint32_t combination) {
  volatile uint32_t res = 0;
  volatile uint32_t inputList = palInputMask;
  volatile uint32_t mask = 1;
  for(uint8_t i = 0; i<activeInputs; i++) {

    while((inputList & 1) == 0) {
      inputList >>= 1;
      res |= mask;
      mask <<= 1;
    }
    if((combination & 1) == 1) res |= mask;
    combination >>= 1;
    mask <<= 1;
    inputList >>= 1;
  }

  return res;
}
PALDump::PALWord PALDump::getPort(uint32_t combination) {
  volatile uint8_t p3Template;
  PALWord res;
  res.port = 0xffff;
  switch(currentPALType) {
    case PAL16L8:
      *HWPort1 = LO8(combination);
      *HWPort2 = HI8(combination);
      p3Template = (HI8(combination)>>1) & DIR_PAL16L8;
      break;
    case PAL20L8:
      uint16_t ccopy;
      LO8(ccopy) = BYTE2(combination);
      HI8(ccopy) = BYTE3(combination);
      ccopy <<= 3;
      *HWPort1 = BYTE1(combination);
      *HWPort2 = BYTE2(combination);
      p3Template = HI8(ccopy);
      break;
  }

  DDR(PROBEPORT) |= 1<<PROBEPIN;

  /* Intentionally made with no loops for better performance! */
  // bit 7
  *HWPort3 = 0b01111111 & p3Template;
  DDR(*HWPort3) = 0b01111111 & DIR_PAL16L8;
  PROBEPORT |= 1<<PROBEPIN;
  NOP;
  if((PIN(*HWPort3)&0b10000000) == 0) res.oe8 = 0;
  PROBEPORT &= (1<<PROBEPIN)^255;
  NOP;
  if((PIN(*HWPort3)&0b10000000) != 0) res.output8 = 0;

  // bit 6
  *HWPort3 = 0b10111111 & p3Template;
  DDR(*HWPort3) = 0b10111111 & DIR_PAL16L8;
  PROBEPORT |= 1<<PROBEPIN;
  NOP;
  if((PIN(*HWPort3)&0b01000000) == 0) res.oe7 = 0;
  PROBEPORT &= (1<<PROBEPIN)^255;
  NOP;
  if((PIN(*HWPort3)&0b01000000) != 0) res.output7 = 0;

  // bit 5
  *HWPort3 = 0b11011111 & p3Template;
  DDR(*HWPort3) = 0b11011111 & DIR_PAL16L8;
  PROBEPORT |= 1<<PROBEPIN;
  NOP;
  if((PIN(*HWPort3)&0b00100000) == 0) res.oe6 = 0;
  PROBEPORT &= (1<<PROBEPIN)^255;
  NOP;
  if((PIN(*HWPort3)&0b00100000) != 0) res.output6 = 0;

  // bit 4
  *HWPort3 = 0b11101111 & p3Template;
  DDR(*HWPort3) = 0b11101111 & DIR_PAL16L8;
  PROBEPORT |= 1<<PROBEPIN;
  NOP;
  if((PIN(*HWPort3)&0b00010000) == 0) res.oe5 = 0;
  PROBEPORT &= (1<<PROBEPIN)^255;
  NOP;
  if((PIN(*HWPort3)&0b00010000) != 0) res.output5 = 0;

  // bit 3
  *HWPort3 = 0b11110111 & p3Template;
  DDR(*HWPort3) = 0b11110111 & DIR_PAL16L8;
  PROBEPORT |= 1<<PROBEPIN;
  NOP;
  if((PIN(*HWPort3)&0b00001000) == 0) res.oe4 = 0;
  PROBEPORT &= (1<<PROBEPIN)^255;
  NOP;
  if((PIN(*HWPort3)&0b00001000) != 0) res.output4 = 0;

  // bit 2
  *HWPort3 = 0b11111011 & p3Template;
  DDR(*HWPort3) = 0b11111011 & DIR_PAL16L8;
  PROBEPORT |= 1<<PROBEPIN;
  NOP;
  if((PIN(*HWPort3)&0b00000100) == 0) res.oe3 = 0;
  PROBEPORT &= (1<<PROBEPIN)^255;
  NOP;
  if((PIN(*HWPort3)&0b00000100) != 0) res.output3 = 0;

  // bit 1
  *HWPort3 = 0b11111101 & p3Template;
  DDR(*HWPort3) = 0b11111101 & DIR_PAL16L8;
  PROBEPORT |= 1<<PROBEPIN;
  NOP;
  if((PIN(*HWPort3)&0b00000010) == 0) res.oe2 = 0;
  PROBEPORT &= (1<<PROBEPIN)^255;
  NOP;
  if((PIN(*HWPort3)&0b00000010) != 0) res.output2 = 0;

  // bit 0
  *HWPort3 = 0b11111110 & p3Template;
  DDR(*HWPort3) = 0b11111110 & DIR_PAL16L8;
  PROBEPORT |= 1<<PROBEPIN;
  NOP;
  if((PIN(*HWPort3)&0b00000001) == 0) res.oe1 = 0;
  PROBEPORT &= (1<<PROBEPIN)^255;
  NOP;
  if((PIN(*HWPort3)&0b00000001) != 0) res.output1 = 0;

  DDR(PROBEPORT) &= (1<<PROBEPIN) ^ 255;
  DDR(*HWPort3) = 0;
  *HWPort3 = 0;

  return res;
}
PALDump::palPinType PALDump::convertSumToPinType(uint32_t sum) {
  switch(currentPALType) {
    case PAL16L8:
      switch(sum) {
        case 0:
          return Output;
        case 65536:
          return Input;
        default:
          return Buffered;
      }
      break;
    case PAL20L8:
      switch(sum) {
        case 0:
          return Output;
        case 1048576:
          return Input;
        default:
          return Buffered;
      }
      break;
    
  }
}
PALDump::palPinType *PALDump::getPALConfig() {
  for(uint8_t pin = 0; pin<8; pin++) {
    palPinConfigCopy[pin] = palPinConfig[pin];
  }
  return (palPinType *)palPinConfigCopy;
}
const char *PALDump::pinType(PALDump::palPinType t) {
  const char * names[4] = {"Output", "Buffered Output", "Input", "Unknown"};
  return names[(uint8_t)t];
}
