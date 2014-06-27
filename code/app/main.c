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


/*===========================================================================*/
/* Thread pointers.                                                          */
/*===========================================================================*/

Thread* pThreadBDU = NULL;
Thread* pThreadSDU = NULL;
Thread* pThreadSensors = NULL;
Thread* pThreadKnock = NULL;
Thread* pThreadCAN = NULL;

monitor_t monitoring = {0,0,0,0,0,0,0,100};

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

const CANConfig cancfg = {
  CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(6)
};

void iwdgGptCb(GPTDriver *gptp)
{
  (void) gptp;
  systime_t time = chTimeNow();
  IWDG->KR = ((uint16_t)0xAAAA);
  time = time - chTimeNow();
  monitoring.irq += time;
  chThdSelf()->runtime -= time;
}

GPTConfig gpt1Cfg =
{
  100000,      /* timer clock.*/
  iwdgGptCb,  /* Timer callback.*/
  0
};

/*
 * DAC config
 */
static const DACConfig daccfg1 = {
  DAC_DHRM_12BIT_RIGHT, /* data holding register mode */
  0 /* DAC CR flags */
};

const SerialConfig uartCfg =
{
 10400, // bit rate
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

WORKING_AREA(waThreadCAN, 256);
msg_t ThreadCAN(void *p)
{
  EventListener el;
  CANRxFrame rxmsg;

  (void)p;
  chRegSetThreadName("CAN Bus");
  chEvtRegister(&CAND1.rxfull_event, &el, 0);

  while(!chThdShouldTerminate()) {
    if (chEvtWaitAnyTimeout(ALL_EVENTS, TIME_IMMEDIATE) == 0) {
      chThdSleepMilliseconds(2);
      continue;
    }
    while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == RDY_OK) {
      /* Process message.*/
    }
  }
  chEvtUnregister(&CAND1.rxfull_event, &el);
  return 0;
}

/*
 * USB Bulk thread.
 */
WORKING_AREA(waThreadBDU, 512);
msg_t ThreadBDU(void *arg)
{
  EventListener el1;
  flagsmask_t flags;
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
WORKING_AREA(waThreadSDU, 1024);
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

    read = sdReadTimeout(&SD1, buffer, sizeof(buffer), TIME_IMMEDIATE);
    if (read > 0)
      sdWriteTimeout(&SDU1, buffer, read, MS2ST(100));

    chThdSleepMilliseconds(1);

    pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));
  }
  return 0;
}

/*
 * Sensors thread.
 */
WORKING_AREA(waThreadSensors, 32);
msg_t ThreadSensors(void *arg)
{
  (void)arg;
  chRegSetThreadName("Sensors");
  while (TRUE)
  {
    chThdSleepMilliseconds(50);
    //if (TIM3->DIER & ~(TIM_DIER_CC1IE | TIM_DIER_CC2IE)) {
      //TIM3->CNT = 0;
      TIM3->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE;
    //}
  }
  return 0;
}

/*
 * Knock processing thread.
 */
WORKING_AREA(waThreadKnock, 128);
msg_t ThreadKnock(void *arg)
{
  (void)arg;
  chRegSetThreadName("Knock");

  q15_t maxValue = 0;
  uint32_t maxIndex = 0;

  /* Initialize the CFFT/CIFFT module */
  arm_cfft_radix4_instance_q15 S;
  arm_cfft_radix4_init_q15(&S, FFT_SIZE, 0, 1);

  while (TRUE)
  {
    while (!knockDataReady) chThdSleepMilliseconds(2);
    knockDataReady = false;

    /* Process the data through the CFFT/CIFFT module */
    arm_cfft_radix4_q15(&S, data_knock);

    /* Process the data through the Complex Magnitude Module for
    calculating the magnitude at each bin */
    memset(output_knock, 0, sizeof(output_knock)); // Clear buffer
    arm_cmplx_mag_q15(data_knock, output_knock, FFT_SIZE); // Calculate magnitude
    arm_max_q15(output_knock, FFT_SIZE, &maxValue, &maxIndex); // Find max magnitude

    sensors_data.knock_value = maxValue;
    sensors_data.knock_freq = (SAMPLING_RATE*maxIndex)/FFT_SIZE;
  }
  return 0;
}

/* Check if tp was the previous thread */
#define RUNNING(tp) (uint16_t)((tp == pThreadMonitor->p_next) << 15)

/*
 * CPU Load Monitoring thread.
 */
WORKING_AREA(waThreadMonitor, 256);
msg_t ThreadMonitor(void *arg)
{
  (void)arg;
  chRegSetThreadName("Monitor");
  uint32_t  run_offset, irq_ticks, total_ticks;
  Thread* pThreadMonitor = chThdSelf();

  DWT->CTRL |= DWT_CTRL_EXCEVTENA_Msk;

  while (TRUE)
  {
	chSysLock();

	pThreadBDU->runtime = 0;
	pThreadSDU->runtime = 0;
	pThreadCAN->runtime = 0;
	pThreadKnock->runtime = 0;
	pThreadSensors->runtime = 0;
	pThreadMonitor->runtime = 0;
	chSysGetIdleThread()->runtime = 0;

    pThreadBDU->irqtime = 0;
    pThreadSDU->irqtime = 0;
    pThreadCAN->irqtime = 0;
    pThreadKnock->irqtime = 0;
    pThreadSensors->irqtime = 0;
    pThreadMonitor->irqtime = 0;
    chSysGetIdleThread()->irqtime = 0;

	run_offset = DWT->CYCCNT;

	chSysUnlock();

	/* Populate load data */
	chThdSleepMilliseconds(500);

	chSysLock();

	/* Convert to systick time base */
	total_ticks = (DWT->CYCCNT - run_offset) / (STM32_SYSCLK/CH_FREQUENCY);
	irq_ticks = pThreadBDU->irqtime
	    +pThreadSDU->irqtime
	    +pThreadCAN->irqtime
	    +pThreadKnock->irqtime
	    +pThreadSensors->irqtime
	    +pThreadMonitor->irqtime
	    +chSysGetIdleThread()->irqtime;

	chSysUnlock();

	monitoring.bdu = ((pThreadBDU->runtime*10000)/total_ticks) | RUNNING(pThreadBDU);
	monitoring.sdu = ((pThreadSDU->runtime*10000)/total_ticks) | RUNNING(pThreadSDU);
	monitoring.can = ((pThreadCAN->runtime*10000)/total_ticks) | RUNNING(pThreadCAN);
	monitoring.knock = ((pThreadKnock->runtime*10000)/total_ticks) | RUNNING(pThreadKnock);
	monitoring.sensors = ((pThreadSensors->runtime*10000)/total_ticks) | RUNNING(pThreadSensors);
	monitoring.monitor = ((pThreadMonitor->runtime*10000)/total_ticks) | RUNNING(pThreadMonitor);
	monitoring.idle = (((chSysGetIdleThread()->runtime*10000)/total_ticks)) | RUNNING(chSysGetIdleThread());
	monitoring.irq = ((irq_ticks*10000)/total_ticks);
  }
  return 0;
}


/*
 * Application entry point.
 */
int main(void)
{
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  driversInit();
  chSysInit();

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
  RCC->CFGR2 &= ~RCC_CFGR2_ADCPRE12 | ~RCC_CFGR2_ADCPRE34; // Erase registers
  RCC->CFGR2 |= RCC_CFGR2_ADCPRE12_DIV128 | RCC_CFGR2_ADCPRE34_DIV32; // Set prescalers
  sdStart(&SD1, &uartCfg);
  usbStart(serusbcfg.usbp, &usbcfg);
  sduStart(&SDU1, &serusbcfg);
  bduStart(&BDU1, &bulkusbcfg);
  canStart(&CAND1, &cancfg);
  dacStart(&DACD1, &daccfg1);
  adcStart(&ADCD1, NULL);
  adcStart(&ADCD3, NULL);
  timcapStart(&TIMCAPD3, &tc_conf);

  /* ADC 3 Ch1 Offset. -2048 */
  ADC3->OFR1 = ADC_OFR1_OFFSET1_EN | ((1 << 26) & ADC_OFR1_OFFSET1_CH) | 0x0800;

  dacConvert(&DACD1, 0x800);
  adcStartConversion(&ADCD1, &adcgrpcfg_sensors, samples_sensors, ADC_GRP1_BUF_DEPTH);
  adcStartConversion(&ADCD3, &adcgrpcfg_knock, samples_knock, ADC_GRP2_BUF_DEPTH);
  timcapEnable(&TIMCAPD3);

  /*
   * Creates the threads.
   */
  pThreadBDU = chThdCreateStatic(waThreadBDU, sizeof(waThreadBDU), NORMALPRIO, ThreadBDU, NULL);
  pThreadSDU = chThdCreateStatic(waThreadSDU, sizeof(waThreadSDU), NORMALPRIO, ThreadSDU, NULL);
  pThreadSensors = chThdCreateStatic(waThreadSensors, sizeof(waThreadSensors), HIGHPRIO, ThreadSensors, NULL);
  pThreadKnock = chThdCreateStatic(waThreadKnock, sizeof(waThreadKnock), NORMALPRIO, ThreadKnock, NULL);
  pThreadCAN = chThdCreateStatic(waThreadCAN, sizeof(waThreadCAN), NORMALPRIO, ThreadCAN, NULL);
  /* Create last as it uses pointers from above */
  chThdCreateStatic(waThreadMonitor, sizeof(waThreadMonitor), NORMALPRIO+1, ThreadMonitor, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (TRUE)
  {
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
