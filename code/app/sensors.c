#include "sensors.h"
#include "limits.h"

/*===========================================================================*/
/* Variables                                                                 */
/*===========================================================================*/

adcsample_t samples_sensors[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
adcsample_t samples_knock[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];
uint8_t output_knock[SPECTRUM_SIZE];

sensors_t sensors_data = {0x0F,0x0F,0x0F0F,0x0F0F,0x0F0F,0x0F0F,0x0F0F,0x0F0F,0x0F0F,0x0F0F};
uint8_t TIM3CC1CaptureNumber, TIM3CC2CaptureNumber;
uint16_t TIM3CC1ReadValue1, TIM3CC1ReadValue2;
uint16_t TIM3CC2ReadValue1, TIM3CC2ReadValue2;

/*===========================================================================*/
/* CallBacks                                                                 */
/*===========================================================================*/

void captureOverflowCb(TIMCAPDriver *timcapp)
{
  (void)timcapp;

  if ((TIM3->DIER & TIM_DIER_CC1IE)
      && TIM3CC1CaptureNumber == 0) {
    TIM3->DIER &= ~TIM_DIER_CC1IE;
    sensors_data.freq1 = 0;
  }

  if ((TIM3->DIER & TIM_DIER_CC2IE)
      && TIM3CC2CaptureNumber == 0) {
    TIM3->DIER &= ~TIM_DIER_CC2IE;
    sensors_data.freq2 = 0;
  }

}

void capture1Cb(TIMCAPDriver *timcapp)
{
  (void)timcapp;

  if(TIM3CC1CaptureNumber == 0)
  {
    /* Get the Input Capture value */
    TIM3CC1ReadValue1 = TIM3->CCR1;
    TIM3CC1CaptureNumber = 1;
  }
  else if(TIM3CC1CaptureNumber == 1)
  {
      uint32_t Capture;
      /* Get the Input Capture value */
      TIM3CC1ReadValue2 = TIM3->CCR1;

      /* Capture computation */
      if (TIM3CC1ReadValue2 > TIM3CC1ReadValue1)
      {
          Capture = ((uint32_t)TIM3CC1ReadValue2 - (uint32_t)TIM3CC1ReadValue1);
      }
      else
      {
          Capture = (((uint32_t)TIM3CC1ReadValue2 + 0x10000) - (uint32_t)TIM3CC1ReadValue1);
      }

      /* Frequency computation */
      sensors_data.freq1 = (tc_conf.frequency / Capture);

      TIM3CC1ReadValue1 = TIM3CC1ReadValue2;
      TIM3CC1CaptureNumber = 0;

      /* Disable CC1 interrupt */
      TIM3->DIER &= ~TIM_DIER_CC1IE;
  }
}

void capture2Cb(TIMCAPDriver *timcapp)
{
  (void)timcapp;
  if(TIM3CC2CaptureNumber == 0)
  {
    /* Get the Input Capture value */
    TIM3CC2ReadValue1 = TIM3->CCR2;
    TIM3CC2CaptureNumber = 1;
  }
  else if(TIM3CC2CaptureNumber == 1)
  {
      uint32_t Capture;
      /* Get the Input Capture value */
      TIM3CC2ReadValue2 = TIM3->CCR2;

      /* Capture computation */
      if (TIM3CC2ReadValue2 > TIM3CC2ReadValue1)
      {
          Capture = ((uint32_t)TIM3CC2ReadValue2 - (uint32_t)TIM3CC2ReadValue1);
      }
      else
      {
          Capture = (((uint32_t)TIM3CC2ReadValue2 + 0x10000) - (uint32_t)TIM3CC2ReadValue1);
      }

      /* Frequency computation */
      sensors_data.freq2 = (tc_conf.frequency / Capture);

      TIM3CC2ReadValue1 = TIM3CC2ReadValue2;
      TIM3CC2CaptureNumber = 0;

      /* Disable CC2 interrupt */
      TIM3->DIER &= ~TIM_DIER_CC2IE;
  }
}

/* Every 64 samples at 965Hz each, triggers at around 15Hz */
void sensorsCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
  (void)adcp;

  chSysLockFromISR();
  allocSendSamplesI(&sensorsMb, (void*)buffer, n);
  chSysUnlockFromISR();
}

/* Every 1024 samples at 117.263KHz each, triggers at around 114Hz */
void knockCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
  (void)adcp;

  if (samples_knock != buffer) {
    /* Ignore half buffer interrupt */
    return;
  }

  // Do FFT + Mag in a dedicated thread

  chSysLockFromISR();
  allocSendSamplesI(&knockMb, (void*)buffer, n);
  chSysUnlockFromISR();
}

/*===========================================================================*/
/* Configs                                                                   */
/*===========================================================================*/

TIMCAPConfig tc_conf = {
   {TIMCAP_INPUT_ACTIVE_HIGH,
    TIMCAP_INPUT_ACTIVE_HIGH,
    TIMCAP_INPUT_DISABLED,
    TIMCAP_INPUT_DISABLED},
   100000, /* TIM3 Runs at 36Mhz max. (1/100000)*65536 = 0.65s Max */
   {capture1Cb, capture2Cb, NULL, NULL},
   captureOverflowCb,
   0
};

/* ADC12 Clk is 72Mhz/128 562KHz  */
const ADCConversionGroup adcgrpcfg_sensors = {
  TRUE,
  ADC_GRP1_NUM_CHANNELS,
  sensorsCallback,
  NULL,
  ADC_CFGR_CONT,                        /* CFGR    */
  ADC_TR(0, 4095),          /* TR1     */
  ADC_CCR_TSEN | ADC_CCR_VBATEN, /* CCR     */
  {                         /* SMPR[2] */
    ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_181P5) | /* Sampling rate = 562000/(61.5+12.5) = 2.9KHz  */
    ADC_SMPR1_SMP_AN8(ADC_SMPR_SMP_181P5) | /* 7.56Khz for 3 channels = 965Hz */
    ADC_SMPR1_SMP_AN9(ADC_SMPR_SMP_181P5),
    0
  },
  {                         /* SQR[4]  */
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN7) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN8) |
    ADC_SQR1_SQ3_N(ADC_CHANNEL_IN9) | 0,
    0,
    0,
    0
  }
};


/* ADC34 Clk is 72Mhz/1 72Mhz  */
const ADCConversionGroup adcgrpcfg_knock = {
  TRUE,
  ADC_GRP2_NUM_CHANNELS,
  knockCallback,
  NULL,
  ADC_CFGR_CONT | ADC_CFGR_ALIGN,    /* CFGR - Align result to left (convert 12 to 16 bits) */
  ADC_TR(0, 4095),                  /* TR1     */
  0,    /* CCR     */
  {                                 /* SMPR[2] */
    ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_601P5),  /* Sampling rate = 72000000/(601.5+12.5) = 117.263Khz  */
    0,
  },
  {                                 /* SQR[4]  */
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1),
    0,
    0,
    0
  }
};

uint16_t calculateTpFromMillivolt(uint16_t AnMin, uint16_t AnMax, uint16_t AnVal)
{
  const uint16_t tpsMin = 0;
  const uint16_t tpsMax = 10000;

  if (AnVal <= AnMin)
    return tpsMin;
  else if (AnVal >= AnMax)
    return tpsMax;

  return map(AnVal - AnMin, 0, AnMax - AnMin, tpsMin, tpsMax);
}
