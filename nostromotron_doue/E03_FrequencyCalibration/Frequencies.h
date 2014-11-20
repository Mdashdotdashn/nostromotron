
#ifndef _SYNTH_DATA_H_
#define _SYNTH_DATA_H_

#include <inttypes.h>
#include <avr/pgmspace.h>
#include "Fixed.h"

extern const fixed *oscShapeTable[];
extern const uint8_t oscShapeCount;
extern const uint16_t oscShapeLength[];

extern const uint16_t sIntervalToFreqFactorSize;

extern const float sNoteToFrequency[];
extern const float sIntervalToFreqFactor[];

#endif


