#ifndef COMMON_H
#define COMMON_H

#include "ch.h"
#include "hal.h"
#include "protocol.h"

#define CCM_FUNC __attribute__((section(".ram4_init.code")))
//#define CCM_FUNC

inline uint32_t leToInt(uint8_t *ptr);
inline uint32_t beToInt(uint8_t *ptr);
uint8_t checksum(const uint8_t *data, uint8_t length);

inline bool getSwitch1(void);
inline int map(int x, int in_min, int in_max, int out_min, int out_max);

void klineInit(void);
bool fiveBaudInit(SerialDriver *sd);
void setLineCoding(cdc_linecoding_t* lcp, SerialDriver *sdp, SerialConfig* scp);

bool vbatDetect(void);

#endif
