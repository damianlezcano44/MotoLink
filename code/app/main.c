/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <stdio.h>
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "usb_config.h"
#include "commands.h"
#include "sensors.h"
#include "innovate.h"

/*===========================================================================*/
/* Macros                                                                    */
/*===========================================================================*/

/* Check if tp was the previous thread */
#define RUNNING(tp) (uint16_t)((tp == pThreadMonitor->p_next) << 15)
#define FREQIN_INTERVAL MS2ST(50)

/*===========================================================================*/
/* Thread pointers.                                                          */
/*===========================================================================*/

thread_t* pThreadSER2 = NULL;
thread_t* pThreadBDU = NULL;
thread_t* pThreadSDU = NULL;
thread_t* pThreadADC = NULL;
thread_t* pThreadKnock = NULL;
thread_t* pThreadCAN = NULL;
thread_t* pThreadMonitor = NULL;

monitor_t monitoring = {0,0,0,0,0,0,0,0,100};

/*===========================================================================*/
/* Structs                                                                   */
/*===========================================================================*/

static virtual_timer_t vt_freqin;

/*===========================================================================*/
/* CallBacks                                                                 */
/*===========================================================================*/

void iwdgGptCb(GPTDriver *gptp)
{
  (void) gptp;
  iwdgReset(&IWDGD);
}

void freqin_vthandler(void *arg)
{
  (void) arg;
  TIM3->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE;

  chSysLockFromISR();
  chVTSetI(&vt_freqin, FREQIN_INTERVAL, freqin_vthandler, 0);
  chSysUnlockFromISR();
}

/*===========================================================================*/
/* Configs                                                                   */
/*===========================================================================*/

const CANConfig cancfg = {
  CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(6)
};

GPTConfig gpt1Cfg =
{
  100000,      /* timer clock.*/
  iwdgGptCb,  /* Timer callback.*/
  0,
  0
};

static const DACConfig daccfg1 = {
  DAC_DHRM_12BIT_RIGHT, /* data holding register mode */
  0 /* DAC CR flags */
};

const SerialConfig uart1Cfg =
{
 10400, // bit rate
 0,
 USART_CR2_STOP1_BITS,
 0
};

const SerialConfig uart2Cfg =
{
 19200, // bit rate
 0,
 USART_CR2_STOP1_BITS,
 0
};

PWMConfig pwmcfg = {
  10000,    /* 10kHz PWM clock frequency.   */
  50,      /* Initial PWM period 10mS.       */
  NULL,
  {
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL}
  },
  0,
  0
};

/*===========================================================================*/
/* Threads                                                                   */
/*===========================================================================*/

THD_WORKING_AREA(waThreadCAN, 256);
msg_t ThreadCAN(void *p)
{
  event_listener_t el;
  CANRxFrame rxmsg;

  (void)p;
  chRegSetThreadName("CAN Bus");
  chEvtRegister(&CAND1.rxfull_event, &el, 0);

  while(!chThdShouldTerminateX()) {
    if (chEvtWaitAnyTimeout(ALL_EVENTS, TIME_IMMEDIATE) == 0) {
      chThdSleepMilliseconds(2);
      continue;
    }
    while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == MSG_OK) {
      /* Process message.*/
    }
  }
  chEvtUnregister(&CAND1.rxfull_event, &el);
  return 0;
}

/*
 * USB Bulk thread.
 */
THD_WORKING_AREA(waThreadBDU, 512);
msg_t ThreadBDU(void *arg)
{
  event_listener_t el1;
  eventmask_t flags;
  (void)arg;
  chRegSetThreadName("BDU");
  uint16_t idle_duty = 0;

  chEvtRegisterMask(chnGetEventSource(&BDU1), &el1, ALL_EVENTS);

  while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);
  while(BDU1.state != BDU_READY) chThdSleepMilliseconds(10);

  while (TRUE)
  {
    chEvtWaitAnyTimeout(ALL_EVENTS, TIME_IMMEDIATE);
    flags = chEvtGetAndClearFlags(&el1);

    idle_duty = usbConnected() ? 500 : 0;

    pwmEnableChannel(&PWMD2, LED_BLUE_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, idle_duty));

    if (flags & CHN_INPUT_AVAILABLE)
    {
      pwmEnableChannel(&PWMD2, LED_BLUE_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));
      readCommand((BaseChannel *)&BDU1);
    }
    else
     chThdSleepMilliseconds(2);
  }
  return 0;
}

/*
 * USB Serial thread.
 */
THD_WORKING_AREA(waThreadSDU, 1024);
msg_t ThreadSDU(void *arg)
{
  (void)arg;
  uint8_t buffer[SERIAL_BUFFERS_SIZE/2];
  uint8_t buffer_check[SERIAL_BUFFERS_SIZE/2];
  size_t read, i;
  chRegSetThreadName("SDU");

  while(SDU1.state != SDU_READY) chThdSleepMilliseconds(10);
  while(SD1.state != SD_READY) chThdSleepMilliseconds(10);

  while (TRUE) {

    pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 1000));
    read = sdReadTimeout(&SDU1, buffer, sizeof(buffer), TIME_IMMEDIATE);
    if (read > 0)
    {
      pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));
      sdWriteTimeout(&SD1, buffer, read, MS2ST(100));
      sdReadTimeout(&SD1, buffer_check, read, MS2ST(10)); // Read back what we wrote
      for (i=0; i<sizeof(buffer_check); i++)
      {
    	  if (buffer[i] != buffer_check[i])
    	  {
    		  // Echo check has failed
    	  }
      }
    }

    pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 1000));
    read = sdReadTimeout(&SD1, buffer, sizeof(buffer), TIME_IMMEDIATE);
    if (read > 0)
    {
      pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));
      sdWriteTimeout(&SDU1, buffer, read, MS2ST(100));
    }

    chThdSleepMilliseconds(1);
  }
  return 0;
}

/*
 * Sensors thread.
 */
THD_WORKING_AREA(waThreadADC, 64);
msg_t ThreadADC(void *arg)
{
  (void)arg;
  chRegSetThreadName("Sensors");

  adcsample_t * sensorsDataPtr;
  size_t n;
  uint16_t i, pos;
  uint32_t an[3] = {0, 0, 0};
  while (TRUE)
  {
    while (!recvFreeSamples(&sensorsMb, (void*)&sensorsDataPtr, &n))
      chThdSleepMilliseconds(5);

    /* Filtering */
    for (i = 0; i < (n/ADC_GRP1_NUM_CHANNELS); i++)
    {
      pos = i * ADC_GRP1_NUM_CHANNELS;
      an[0] += median_filter1(sensorsDataPtr[pos]);
      an[1] += median_filter2(sensorsDataPtr[pos+1]);
      an[2] += median_filter3(sensorsDataPtr[pos+2]);
    }

    an[0] *= VBAT_RATIO;
    an[1] *= AN_RATIO;
    an[2] *= AN_RATIO;

    /* Averaging */
    an[0] /= (n/ADC_GRP1_NUM_CHANNELS);
    an[1] /= (n/ADC_GRP1_NUM_CHANNELS);
    an[2] /= (n/ADC_GRP1_NUM_CHANNELS);

    sensors_data.an7 = an[0];
    sensors_data.an8 = an[1];
    sensors_data.an9 = an[2];
  }
  return 0;
}

/*
 * Knock processing thread.
 */
static float32_t input[FFT_SIZE*2];
static float32_t output[FFT_SIZE*2];
static float32_t mag_knock[FFT_SIZE/2];
THD_WORKING_AREA(waThreadKnock, 512);
msg_t ThreadKnock(void *arg)
{
  (void)arg;
  chRegSetThreadName("Knock");

  q15_t* knockDataPtr;
  size_t knockDataSize;

  float32_t maxValue = 0;
  uint32_t maxIndex = 0;
  uint16_t i;

  /* Initialize the CFFT/CIFFT module */
  arm_rfft_fast_instance_f32 S1;
  arm_rfft_fast_init_f32(&S1, FFT_SIZE);

  while (TRUE)
  {
    while (!recvFreeSamples(&knockMb, (void*)&knockDataPtr, &knockDataSize))
      chThdSleepMilliseconds(2);

    /* Copy and convert ADC samples */
    for (i=0; i<FFT_SIZE*2; i+=4)
    {
      input[i] = (float32_t)knockDataPtr[i];
      input[i+1] = (float32_t)knockDataPtr[i+1];
      input[i+2] = (float32_t)knockDataPtr[i+2];
      input[i+3] = (float32_t)knockDataPtr[i+3];
    }

    /* Process the data through the CFFT/CIFFT module */
    arm_rfft_fast_f32(&S1, input, output, 0);

    /* Process the data through the Complex Magnitude Module for
    calculating the magnitude at each bin */
    arm_cmplx_mag_f32(output, mag_knock, FFT_SIZE/2); // Calculate magnitude, outputs q2.14
    arm_max_f32(mag_knock, FFT_SIZE/2, &maxValue, &maxIndex); // Find max magnitude

    // Convert 2.14 to 8 Bits unsigned
    for (i=0; i < sizeof(output_knock); i++)
    {
      output_knock[i] = (mag_knock[i]/16384); // 8 bits minus the 2 fractional bits
    }

    sensors_data.knock_value = maxValue;
    sensors_data.knock_freq = (FFT_FREQ*maxIndex)/FFT_SIZE;
  }
  return 0;
}

/* Pointer to the idle thread */
static inline thread_t *chThdGetIdleX(void) {

  return ch.rlist.r_queue.p_prev;
}

/*
 * CPU Load Monitoring thread.
 */
THD_WORKING_AREA(waThreadMonitor, 256);
msg_t ThreadMonitor(void *arg)
{
  (void)arg;
  chRegSetThreadName("Monitor");
  uint32_t  run_offset, irq_ticks, total_ticks;
  pThreadMonitor = chThdGetSelfX();

  DWT->CTRL |= DWT_CTRL_EXCEVTENA_Msk;

  while (TRUE)
  {
	chSysLock();

	pThreadSER2->runtime = 0;
	pThreadBDU->runtime = 0;
	pThreadSDU->runtime = 0;
	pThreadCAN->runtime = 0;
	pThreadKnock->runtime = 0;
	pThreadADC->runtime = 0;
	pThreadMonitor->runtime = 0;
	chThdGetIdleX()->runtime = 0;

	pThreadSER2->irqtime = 0;
    pThreadBDU->irqtime = 0;
    pThreadSDU->irqtime = 0;
    pThreadCAN->irqtime = 0;
    pThreadKnock->irqtime = 0;
    pThreadADC->irqtime = 0;
    pThreadMonitor->irqtime = 0;
    chThdGetIdleX()->irqtime = 0;

	run_offset = DWT->CYCCNT;

	chSysUnlock();

	/* Populate load data */
	chThdSleepMilliseconds(500);

	chSysLock();

	/* Convert to systick time base */
	total_ticks = (DWT->CYCCNT - run_offset) / (STM32_SYSCLK/CH_CFG_ST_FREQUENCY);
	irq_ticks = pThreadSER2->irqtime
	    +pThreadBDU->irqtime
	    +pThreadSDU->irqtime
	    +pThreadCAN->irqtime
	    +pThreadKnock->irqtime
	    +pThreadADC->irqtime
	    +pThreadMonitor->irqtime
	    +chThdGetIdleX()->irqtime;

	chSysUnlock();

	monitoring.ser2 = ((pThreadSER2->runtime*10000)/total_ticks) | RUNNING(pThreadSER2);
	monitoring.bdu = ((pThreadBDU->runtime*10000)/total_ticks) | RUNNING(pThreadBDU);
	monitoring.sdu = ((pThreadSDU->runtime*10000)/total_ticks) | RUNNING(pThreadSDU);
	monitoring.can = ((pThreadCAN->runtime*10000)/total_ticks) | RUNNING(pThreadCAN);
	monitoring.knock = ((pThreadKnock->runtime*10000)/total_ticks) | RUNNING(pThreadKnock);
	monitoring.sensors = ((pThreadADC->runtime*10000)/total_ticks) | RUNNING(pThreadADC);
	monitoring.monitor = ((pThreadMonitor->runtime*10000)/total_ticks) | RUNNING(pThreadMonitor);
	monitoring.idle = (((chThdGetIdleX()->runtime*10000)/total_ticks)) | RUNNING(chThdGetIdleX());
	monitoring.irq = ((irq_ticks*10000)/total_ticks);
  }
  return 0;
}

/*
 * Uart2 thread.
 */
THD_WORKING_AREA(waThreadSER2, 128);
msg_t ThreadSER2(void *arg)
{
  (void)arg;
  uint8_t buffer[SERIAL_BUFFERS_SIZE/2];
  size_t read;
  chRegSetThreadName("SER2");

  while(SD2.state != SD_READY) chThdSleepMilliseconds(10);

  while (TRUE) {

    read = sdReadTimeout(&SD2, buffer, 1, TIME_IMMEDIATE);
    if (read > 0)
    {
      readMtsHeader((BaseChannel *)&SD2, buffer);
    }

    chThdSleepMilliseconds(1);
  }
  return 0;
}

/*===========================================================================*/
/* Main Thread                                                               */
/*===========================================================================*/

int main(void)
{
  /*
   * Start OS and HAL
   */
  halInit();
  driversInit();
  chSysInit();
  setupIPC();

  usbDisconnectBus(serusbcfg.usbp);

  gptStart(&GPTD1, &gpt1Cfg);
  gptStartContinuous(&GPTD1, 20000);

  pwmStart(&PWMD2, &pwmcfg);
  pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));

  /*
   * Initialize extra driver objects.
   */
  sduObjectInit(&SDU1);
  bduObjectInit(&BDU1);

  /*
   * Start peripherals
   */
  sdStart(&SD1, &uart1Cfg);
  sdStart(&SD2, &uart2Cfg);
  usbStart(&USBD1, &usbcfg);
  sduStart(&SDU1, &serusbcfg);
  bduStart(&BDU1, &bulkusbcfg);
  canStart(&CAND1, &cancfg);
  dacStart(&DACD1, &daccfg1);
  dacStart(&DACD2, &daccfg1);
  adcStart(&ADCD1, NULL);
  adcStart(&ADCD3, NULL);
  timcapStart(&TIMCAPD3, &tc_conf);

  /* ADC 3 Ch1 Offset. -2048 */
  ADC3->OFR1 = ADC_OFR1_OFFSET1_EN | ((1 << 26) & ADC_OFR1_OFFSET1_CH) | (2048 & 0xFFF);
  dacConvertOne(&DACD1, 2048); // This sets the offset for the knock ADC opamp.
  dacConvertOne(&DACD2, 2048);
  adcStartConversion(&ADCD1, &adcgrpcfg_sensors, samples_sensors, ADC_GRP1_BUF_DEPTH);
  adcStartConversion(&ADCD3, &adcgrpcfg_knock, samples_knock, ADC_GRP2_BUF_DEPTH);
  timcapEnable(&TIMCAPD3);

  chVTSet(&vt_freqin, FREQIN_INTERVAL, freqin_vthandler, 0);

  /*
   * Creates the threads.
   */
  pThreadBDU = chThdCreateStatic(waThreadBDU, sizeof(waThreadBDU), NORMALPRIO, ThreadBDU, NULL);
  pThreadSDU = chThdCreateStatic(waThreadSDU, sizeof(waThreadSDU), NORMALPRIO, ThreadSDU, NULL);
  pThreadADC = chThdCreateStatic(waThreadADC, sizeof(waThreadADC), HIGHPRIO, ThreadADC, NULL);
  pThreadKnock = chThdCreateStatic(waThreadKnock, sizeof(waThreadKnock), NORMALPRIO, ThreadKnock, NULL);
  pThreadCAN = chThdCreateStatic(waThreadCAN, sizeof(waThreadCAN), NORMALPRIO, ThreadCAN, NULL);
  pThreadSER2 = chThdCreateStatic(waThreadSER2, sizeof(waThreadSER2), NORMALPRIO, ThreadSER2, NULL);
  /* Create last as it uses pointers from above */
  chThdCreateStatic(waThreadMonitor, sizeof(waThreadMonitor), NORMALPRIO+1, ThreadMonitor, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (TRUE)
  {
    while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);

    chThdSleepMilliseconds(100);

    if (usbConnected())
    {
      usbConnectBus(serusbcfg.usbp);
    }
    else
    {
      usbDisconnectBus(serusbcfg.usbp);
    }
  }
}
