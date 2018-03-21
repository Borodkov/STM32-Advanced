#ifndef __SCOPE_H
#define __SCOPE_H

#include <stdint.h>

#define USE_STEMWIN

#define SCOPE_INV_TRIGGER	0xFFFFFFFF
/*10000 samples is base, 2 is the margin for trigger*/
#define DUAL_BUFFER 2
#define LONG_BUFFER 2
#define NUM_BYTES_PER_SAMPLE 2
#define NUM_SAMPLES	(10000*LONG_BUFFER)
#define DEFAULT_TRIGGER_LEVEL	(256/2)
#define TRIGGER_TIMEOUT	100

#ifdef USE_STEMWIN
#define GRAPH_BORDER_L	50
#define SCOPE_STBLUE		((220 << 16) + (169 << 8) + (57 << 0))
#endif

typedef enum {
    TRIGGER_DISABLED = 0,
    TRIGGER_NORMAL = 1,
    TRIGGER_AUTO = 2,
    TRIGGER_SINGLE = 3
} TriggerMode_TypeDef;

typedef struct {
    uint32_t numberOfSamples;
    volatile uint32_t newNumberOfSamples;
    volatile uint16_t triggerLevel;
    uint8_t triggerEdge;
    volatile TriggerMode_TypeDef triggerMode;
} OSC_TypeDef;

extern OSC_TypeDef oscConfig;

uint32_t ScopeFindTrigger(uint32_t triggerEdge, short triggerLevel, short *pData, uint32_t numberOfSamples);
void ScopeScale(short *pDataIn, short *pDataOut, uint32_t inMax, uint32_t outMax, uint32_t numSamples);
uint32_t ScopeProcess(short *pData);
void ScopeLCD_Show(short *pData);
void ScopeLCD_Init(void);

#endif //__SCOPE_H
