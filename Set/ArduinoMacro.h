#pragma once

#ifndef bitRead
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#endif

#ifndef bitSet
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#endif

#ifndef bitClear
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#endif

#ifndef bitToggle
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#endif

#ifndef bitWrite
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet((value), (bit)) : bitClear((value), (bit)))
#endif

#ifndef bit
#define bit(b) (1UL << (b))
#endif
