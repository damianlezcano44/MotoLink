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
#include "chprintf.h"
#include "common.h"
#include "communication.h"

/*===========================================================================*/
/* Config                                                                    */
/*===========================================================================*/

uint8_t reset_flags = FLAG_OK;

static PWMConfig pwmcfg = {
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
/* Generic code.                                                             */
/*===========================================================================*/

// Duty
#define D(x) PWM_PERCENTAGE_TO_WIDTH(&PWMD2, x)

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThreadBlinker, 384);
static msg_t ThreadBlinker(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");

  const uint16_t dimmer[] = {D(0), D(200), D(400), D(600), D(800), D(1000), D(1200), D(1400), D(1600), D(1800), D(2000), D(2200),
                             D(2400), D(2600), D(2800), D(3000),  D(3200), D(3400), D(3600), D(3800), D(4000), D(4200), D(4400),
                             D(4600), D(4800), D(5000), D(5200), D(5400), D(5600), D(5800), D(6000), D(6200), D(6400), D(6600),
                             D(6800), D(7000), D(7200), D(7400), D(7600), D(7800), D(8000), D(7800), D(7600), D(7400), D(7200),
                             D(7000), D(6800), D(6600), D(6400), D(6200), D(6000), D(5800), D(5600), D(5400), D(5200), D(5000),
                             D(4800), D(4600), D(4400), D(4200), D(4000), D(3800), D(3600), D(3400), D(3200), D(3000), D(2800),
                             D(2600), D(2400), D(2200), D(2000), D(1800), D(1600), D(1400), D(1200), D(1000), D(800), D(600),
                             D(400), D(200)};


  TIM2->DIER |= TIM_DIER_UDE; /* Timer Update DMA request */
  if (dmaStreamAllocate(STM32_DMA1_STREAM2, 0, NULL, NULL)) while (1) chThdSleepMilliseconds(20);
  dmaStreamSetPeripheral(STM32_DMA1_STREAM2, &TIM2->CCR3);
  dmaStreamSetMemory0(STM32_DMA1_STREAM2, dimmer);
  dmaStreamSetTransactionSize(STM32_DMA1_STREAM2, sizeof(dimmer)/2);
  dmaStreamSetMode(STM32_DMA1_STREAM2, STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_HWORD
                   | STM32_DMA_CR_EN | STM32_DMA_CR_CIRC | STM32_DMA_CR_DIR_M2P | DMA_CCR_MINC);

  while (TRUE)
  {
    chThdSleepMilliseconds(100);
    if (usbConnected())
      TIM2->PSC = (STM32_TIMCLK1 / 10000) - 1;
    else
      TIM2->PSC = (STM32_TIMCLK1 / 5000) - 1;
  }
  return 0;
}

/*
 * USB Bulk thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThreadBDU, 1024);
static msg_t ThreadBDU(void *arg) {

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
      readCommand((BaseChannel *)&BDU1, reset_flags);
    }
  }
  return 0;
}

/*
 * USB Serial thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThreadSDU, 1024);
static msg_t ThreadSDU(void *arg) {

  uint8_t buffer[16];
  event_listener_t el1;
  eventmask_t flags_usb;
  (void)arg;
  chRegSetThreadName("SDU");
  chEvtRegisterMask(chnGetEventSource(&SDU1), &el1, CHN_INPUT_AVAILABLE);
  while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);

  while(SDU1.state != SDU_READY) chThdSleepMilliseconds(10);

  while (TRUE)
  {
    chEvtWaitOneTimeout(EVENT_MASK(1), TIME_IMMEDIATE);
    flags_usb = chEvtGetAndClearFlags(&el1);

    if (flags_usb & CHN_INPUT_AVAILABLE) { /* Incoming data from USB */

      /* Does nothing with the data */
      chnReadTimeout((BaseChannel *)&SDU1, buffer, sizeof(buffer), MS2ST(5));
    }
    else {
      chThdSleepMilliseconds(10);
    }
  }
  return 0;
}

/*
 * Application entry point.
 */
int main(void) {

  /* Begin charging switch cap */
  palInit(&pal_default_config);

  /*!< Independent Watchdog reset flag */
  if (RCC->CSR & RCC_CSR_IWDGRSTF) {
    /* User App did not start properly */

    reset_flags |= FLAG_IWDRST;
  }

  /*!< Software Reset flag */
  if (RCC->CSR & RCC_CSR_SFTRSTF) {
    /* Bootloader called by user app */

    reset_flags |= FLAG_SFTRST;
  }

  /* Check user app */
  if (checkUserCode(USER_APP_ADDR) != 1) {
    reset_flags |= FLAG_NOAPP;
  }

  /* Give enough time for the switch's debounce cap to charge
   * Given the 25<->55k pullup, 100nf cap, 0.8v threshold voltage
   * it takes 0.7ms<->1.5ms to reach the High level voltage */
  uint32_t i = 0;
  while (i++ < 100000) // 1.4ms * 3
  {
    asm ("nop");
  }

  /* Check boot switch */
  if (getSwitch1()) {
    reset_flags |= FLAG_SWITCH;
  }

  /*!< Remove hardware reset flags */
  RCC->CSR |= RCC_CSR_RMVF;

  if (reset_flags == FLAG_OK)
  {
    jumpToUser(USER_APP_ADDR);
    while (1);
  }

  halInit();
  chSysInit();

  pwmStart(&PWMD2, &pwmcfg);
  usbStart(&USBD1, &usbcfg);

  bduObjectInit(&BDU1);
  sduObjectInit(&SDU1);

  bduStart(&BDU1, &bulkusbcfg);
  sduStart(&SDU1, &serusbcfg);;

  chThdCreateStatic(waThreadBlinker, sizeof(waThreadBlinker), NORMALPRIO, ThreadBlinker, NULL);
  chThdCreateStatic(waThreadBDU, sizeof(waThreadBDU), NORMALPRIO, ThreadBDU, NULL);
  chThdCreateStatic(waThreadSDU, sizeof(waThreadSDU), NORMALPRIO, ThreadSDU, NULL);

  while (TRUE)    {
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
