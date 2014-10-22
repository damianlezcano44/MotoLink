#ifndef SENSORS_H
#define SENSORS_H

#include "ch.h"
#include "hal.h"
#include "protocol.h"
#include "drivers.h"
#include "arm_math.h"
#include "median.h"
#include "ipc.h"

#define ADC_GRP1_NUM_CHANNELS   3
#define ADC_GRP1_BUF_DEPTH      128

#define ADC_GRP2_NUM_CHANNELS   1
#define ADC_GRP2_BUF_DEPTH      4096 /* 2x2048 for continuous FFT1024 */

#define AN_RATIO 1.61 /* 6600mV/4096 voltage divider ratio is 1.5 */
#define VBAT_RATIO 9.0f /* 36300mV/4096 voltage divider ratio is 9 */

#define FFT_SIZE 1024 // 4096-1024-256-64-16 lengths supported by DSP library
#define SAMPLING_RATE 112500

extern adcsample_t samples_sensors[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
extern adcsample_t samples_knock[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];
extern q15_t mag_knock[sizeof(samples_knock)/2];
extern uint8_t output_knock[SPECTRUM_SIZE];

extern const ADCConversionGroup adcgrpcfg_sensors;
extern const ADCConversionGroup adcgrpcfg_knock;

extern sensors_t sensors_data;
extern TIMCAPConfig tc_conf;
extern monitor_t monitoring;

#endif
