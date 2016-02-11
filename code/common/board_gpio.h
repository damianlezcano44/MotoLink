#ifndef _BOARD_GPIO_H_
#define _BOARD_GPIO_H_

#define PORT_KNOCK_OFFSET GPIOA
#define PAD_KNOCK_OFFSET 4

#define PORT_SPI1_SCK GPIOA
#define PAD_SPI1_SCK 5

#define PORT_SPI1_MISO GPIOA
#define PAD_SPI1_MISO 6

#define PORT_SPI1_MOSI GPIOA
#define PAD_SPI1_MOSI 7

#define PORT_USART1_TX GPIOA
#define PAD_USART1_TX 9

#define PORT_USART1_RX GPIOA
#define PAD_USART1_RX 10

#define PORT_USB_DM GPIOA
#define PAD_USB_DM 11

#define PORT_USB_DP GPIOA
#define PAD_USB_DP 12

#define PORT_SYS_JTMS_SWDIO GPIOA
#define PAD_SYS_JTMS_SWDIO 13

#define PORT_SYS_JTCK_SWCLK GPIOA
#define PAD_SYS_JTCK_SWCLK 14

#define PORT_SPI1_NSS GPIOA
#define PAD_SPI1_NSS 15

#define PORT_SCS_TRIG GPIOB
#define PAD_SCS_TRIG 0

#define PORT_KNK_AN GPIOB
#define PAD_KNK_AN 1

#define PORT_USART2_TX GPIOB
#define PAD_USART2_TX 3

#define PORT_USART2_RX GPIOB
#define PAD_USART2_RX 4

#define PORT_VCC_DETECT GPIOB
#define PAD_VCC_DETECT 5

#define PORT_LED1 GPIOB
#define PAD_LED1 6
#define TIM_LED1 TIM4
#define CCR_LED1 CCR1
#define PWMD_LED1 PWMD4
#define ICUD_LED1 ICUD4
#define CHN_LED1 0

#define PORT_LED2 GPIOB
#define PAD_LED2 7
#define TIM_LED2 TIM4
#define CCR_LED2 CCR2
#define PWMD_LED2 PWMD4
#define ICUD_LED2 ICUD4
#define CHN_LED2 1

#define PORT_CAN_RX GPIOB
#define PAD_CAN_RX 8

#define PORT_CAN_TX GPIOB
#define PAD_CAN_TX 9

#define PORT_SPI2_NSS GPIOB
#define PAD_SPI2_NSS 12

#define PORT_SPI2_SCK GPIOB
#define PAD_SPI2_SCK 13

#define PORT_SPI2_MISO GPIOB
#define PAD_SPI2_MISO 14

#define PORT_SPI2_MOSI GPIOB
#define PAD_SPI2_MOSI 15

#define PORT_AN0 GPIOC
#define PAD_AN0 1

#define PORT_AN1 GPIOC
#define PAD_AN1 2

#define PORT_AN2 GPIOC
#define PAD_AN2 3

#define PORT_FREQ2_IN GPIOC
#define PAD_FREQ2_IN 6
#define TIM_FREQ2_IN TIM3
#define CCR_FREQ2_IN CCR1
#define PWMD_FREQ2_IN PWMD3
#define ICUD_FREQ2_IN ICUD3
#define CHN_FREQ2_IN 0

#define PORT_FREQ1_IN GPIOC
#define PAD_FREQ1_IN 7
#define TIM_FREQ1_IN TIM3
#define CCR_FREQ1_IN CCR2
#define PWMD_FREQ1_IN PWMD3
#define ICUD_FREQ1_IN ICUD3
#define CHN_FREQ1_IN 1

#define PORT_USB_CTRL GPIOC
#define PAD_USB_CTRL 8

#define PORT_KLINE_TX GPIOC
#define PAD_KLINE_TX 10

#define PORT_KLINE_RX GPIOC
#define PAD_KLINE_RX 11

#define PORT_KLINE_CS GPIOC
#define PAD_KLINE_CS 12

#define PORT_USB_DETECT GPIOC
#define PAD_USB_DETECT 15

#define PORT_RCC_OSC_IN GPIOF
#define PAD_RCC_OSC_IN 0

#define PORT_RCC_OSC_OUT GPIOF
#define PAD_RCC_OSC_OUT 1

#define PORT_BUTTON1 GPIOF
#define PAD_BUTTON1 4

/* PORT A */
#define VAL_GPIOA_MODER ( \
    PIN_MODE_ANALOG(0) | \
    PIN_MODE_ANALOG(1) | \
    PIN_MODE_ANALOG(2) | \
    PIN_MODE_ANALOG(3) | \
    PIN_MODE_ANALOG(4) | \
    PIN_MODE_ALTERNATE(5) | \
    PIN_MODE_ALTERNATE(6) | \
    PIN_MODE_ALTERNATE(7) | \
    PIN_MODE_ANALOG(8) | \
    PIN_MODE_ALTERNATE(9) | \
    PIN_MODE_ALTERNATE(10) | \
    PIN_MODE_ALTERNATE(11) | \
    PIN_MODE_ALTERNATE(12) | \
    PIN_MODE_ALTERNATE(13) | \
    PIN_MODE_ALTERNATE(14) | \
    PIN_MODE_ALTERNATE(15))

#define VAL_GPIOA_OTYPER ( \
    PIN_OTYPE_PUSHPULL(0) | \
    PIN_OTYPE_PUSHPULL(1) | \
    PIN_OTYPE_PUSHPULL(2) | \
    PIN_OTYPE_PUSHPULL(3) | \
    PIN_OTYPE_PUSHPULL(4) | \
    PIN_OTYPE_PUSHPULL(5) | \
    PIN_OTYPE_PUSHPULL(6) | \
    PIN_OTYPE_PUSHPULL(7) | \
    PIN_OTYPE_PUSHPULL(8) | \
    PIN_OTYPE_OPENDRAIN(9) | \
    PIN_OTYPE_OPENDRAIN(10) | \
    PIN_OTYPE_PUSHPULL(11) | \
    PIN_OTYPE_PUSHPULL(12) | \
    PIN_OTYPE_PUSHPULL(13) | \
    PIN_OTYPE_PUSHPULL(14) | \
    PIN_OTYPE_PUSHPULL(15))

#define VAL_GPIOA_OSPEEDR ( \
    PIN_OSPEED_2M(0) | \
    PIN_OSPEED_2M(1) | \
    PIN_OSPEED_2M(2) | \
    PIN_OSPEED_2M(3) | \
    PIN_OSPEED_2M(4) | \
    PIN_OSPEED_100M(5) | \
    PIN_OSPEED_100M(6) | \
    PIN_OSPEED_100M(7) | \
    PIN_OSPEED_2M(8) | \
    PIN_OSPEED_50M(9) | \
    PIN_OSPEED_50M(10) | \
    PIN_OSPEED_100M(11) | \
    PIN_OSPEED_100M(12) | \
    PIN_OSPEED_2M(13) | \
    PIN_OSPEED_2M(14) | \
    PIN_OSPEED_100M(15))

#define VAL_GPIOA_PUPDR ( \
    PIN_PUPDR_FLOATING(0) | \
    PIN_PUPDR_FLOATING(1) | \
    PIN_PUPDR_FLOATING(2) | \
    PIN_PUPDR_FLOATING(3) | \
    PIN_PUPDR_FLOATING(4) | \
    PIN_PUPDR_FLOATING(5) | \
    PIN_PUPDR_FLOATING(6) | \
    PIN_PUPDR_FLOATING(7) | \
    PIN_PUPDR_FLOATING(8) | \
    PIN_PUPDR_FLOATING(9) | \
    PIN_PUPDR_FLOATING(10) | \
    PIN_PUPDR_FLOATING(11) | \
    PIN_PUPDR_FLOATING(12) | \
    PIN_PUPDR_FLOATING(13) | \
    PIN_PUPDR_FLOATING(14) | \
    PIN_PUPDR_FLOATING(15))

#define VAL_GPIOA_ODR ( \
    PIN_ODR_HIGH(0) | \
    PIN_ODR_HIGH(1) | \
    PIN_ODR_HIGH(2) | \
    PIN_ODR_HIGH(3) | \
    PIN_ODR_HIGH(4) | \
    PIN_ODR_HIGH(5) | \
    PIN_ODR_HIGH(6) | \
    PIN_ODR_HIGH(7) | \
    PIN_ODR_HIGH(8) | \
    PIN_ODR_HIGH(9) | \
    PIN_ODR_HIGH(10) | \
    PIN_ODR_HIGH(11) | \
    PIN_ODR_HIGH(12) | \
    PIN_ODR_HIGH(13) | \
    PIN_ODR_HIGH(14) | \
    PIN_ODR_HIGH(15))

#define VAL_GPIOA_AFRL ( \
    PIN_AFIO_AF(0, 0) | \
    PIN_AFIO_AF(1, 0) | \
    PIN_AFIO_AF(2, 0) | \
    PIN_AFIO_AF(3, 0) | \
    PIN_AFIO_AF(4, 0) | \
    PIN_AFIO_AF(5, 5) | \
    PIN_AFIO_AF(6, 5) | \
    PIN_AFIO_AF(7, 5))

#define VAL_GPIOA_AFRH ( \
    PIN_AFIO_AF(8, 0) | \
    PIN_AFIO_AF(9, 7) | \
    PIN_AFIO_AF(10, 7) | \
    PIN_AFIO_AF(11, 14) | \
    PIN_AFIO_AF(12, 14) | \
    PIN_AFIO_AF(13, 0) | \
    PIN_AFIO_AF(14, 0) | \
    PIN_AFIO_AF(15, 5))

/* PORT B */
#define VAL_GPIOB_MODER ( \
    PIN_MODE_OUTPUT(0) | \
    PIN_MODE_ANALOG(1) | \
    PIN_MODE_ANALOG(2) | \
    PIN_MODE_ALTERNATE(3) | \
    PIN_MODE_ALTERNATE(4) | \
    PIN_MODE_INPUT(5) | \
    PIN_MODE_ALTERNATE(6) | \
    PIN_MODE_ALTERNATE(7) | \
    PIN_MODE_ALTERNATE(8) | \
    PIN_MODE_ALTERNATE(9) | \
    PIN_MODE_ANALOG(10) | \
    PIN_MODE_ANALOG(11) | \
    PIN_MODE_OUTPUT(12) | \
    PIN_MODE_ALTERNATE(13) | \
    PIN_MODE_ALTERNATE(14) | \
    PIN_MODE_ALTERNATE(15))

#define VAL_GPIOB_OTYPER ( \
    PIN_OTYPE_PUSHPULL(0) | \
    PIN_OTYPE_PUSHPULL(1) | \
    PIN_OTYPE_PUSHPULL(2) | \
    PIN_OTYPE_OPENDRAIN(3) | \
    PIN_OTYPE_OPENDRAIN(4) | \
    PIN_OTYPE_PUSHPULL(5) | \
    PIN_OTYPE_PUSHPULL(6) | \
    PIN_OTYPE_PUSHPULL(7) | \
    PIN_OTYPE_PUSHPULL(8) | \
    PIN_OTYPE_PUSHPULL(9) | \
    PIN_OTYPE_PUSHPULL(10) | \
    PIN_OTYPE_PUSHPULL(11) | \
    PIN_OTYPE_PUSHPULL(12) | \
    PIN_OTYPE_PUSHPULL(13) | \
    PIN_OTYPE_OPENDRAIN(14) | \
    PIN_OTYPE_PUSHPULL(15))

#define VAL_GPIOB_OSPEEDR ( \
    PIN_OSPEED_2M(0) | \
    PIN_OSPEED_2M(1) | \
    PIN_OSPEED_2M(2) | \
    PIN_OSPEED_50M(3) | \
    PIN_OSPEED_50M(4) | \
    PIN_OSPEED_2M(5) | \
    PIN_OSPEED_50M(6) | \
    PIN_OSPEED_50M(7) | \
    PIN_OSPEED_50M(8) | \
    PIN_OSPEED_50M(9) | \
    PIN_OSPEED_2M(10) | \
    PIN_OSPEED_2M(11) | \
    PIN_OSPEED_100M(12) | \
    PIN_OSPEED_100M(13) | \
    PIN_OSPEED_100M(14) | \
    PIN_OSPEED_100M(15))

#define VAL_GPIOB_PUPDR ( \
    PIN_PUPDR_FLOATING(0) | \
    PIN_PUPDR_FLOATING(1) | \
    PIN_PUPDR_FLOATING(2) | \
    PIN_PUPDR_FLOATING(3) | \
    PIN_PUPDR_FLOATING(4) | \
    PIN_PUPDR_PULLUP(5) | \
    PIN_PUPDR_FLOATING(6) | \
    PIN_PUPDR_FLOATING(7) | \
    PIN_PUPDR_FLOATING(8) | \
    PIN_PUPDR_FLOATING(9) | \
    PIN_PUPDR_FLOATING(10) | \
    PIN_PUPDR_FLOATING(11) | \
    PIN_PUPDR_FLOATING(12) | \
    PIN_PUPDR_FLOATING(13) | \
    PIN_PUPDR_FLOATING(14) | \
    PIN_PUPDR_FLOATING(15))

#define VAL_GPIOB_ODR ( \
    PIN_ODR_HIGH(0) | \
    PIN_ODR_HIGH(1) | \
    PIN_ODR_HIGH(2) | \
    PIN_ODR_HIGH(3) | \
    PIN_ODR_HIGH(4) | \
    PIN_ODR_HIGH(5) | \
    PIN_ODR_HIGH(6) | \
    PIN_ODR_HIGH(7) | \
    PIN_ODR_HIGH(8) | \
    PIN_ODR_HIGH(9) | \
    PIN_ODR_HIGH(10) | \
    PIN_ODR_HIGH(11) | \
    PIN_ODR_HIGH(12) | \
    PIN_ODR_HIGH(13) | \
    PIN_ODR_HIGH(14) | \
    PIN_ODR_HIGH(15))

#define VAL_GPIOB_AFRL ( \
    PIN_AFIO_AF(0, 0) | \
    PIN_AFIO_AF(1, 0) | \
    PIN_AFIO_AF(2, 0) | \
    PIN_AFIO_AF(3, 7) | \
    PIN_AFIO_AF(4, 7) | \
    PIN_AFIO_AF(5, 0) | \
    PIN_AFIO_AF(6, 2) | \
    PIN_AFIO_AF(7, 2))

#define VAL_GPIOB_AFRH ( \
    PIN_AFIO_AF(8, 9) | \
    PIN_AFIO_AF(9, 9) | \
    PIN_AFIO_AF(10, 0) | \
    PIN_AFIO_AF(11, 0) | \
    PIN_AFIO_AF(12, 0) | \
    PIN_AFIO_AF(13, 5) | \
    PIN_AFIO_AF(14, 5) | \
    PIN_AFIO_AF(15, 5))

/* PORT C */
#define VAL_GPIOC_MODER ( \
    PIN_MODE_ANALOG(0) | \
    PIN_MODE_ANALOG(1) | \
    PIN_MODE_ANALOG(2) | \
    PIN_MODE_ANALOG(3) | \
    PIN_MODE_ANALOG(4) | \
    PIN_MODE_ANALOG(5) | \
    PIN_MODE_ALTERNATE(6) | \
    PIN_MODE_ALTERNATE(7) | \
    PIN_MODE_OUTPUT(8) | \
    PIN_MODE_ANALOG(9) | \
    PIN_MODE_ALTERNATE(10) | \
    PIN_MODE_ALTERNATE(11) | \
    PIN_MODE_OUTPUT(12) | \
    PIN_MODE_ANALOG(13) | \
    PIN_MODE_ANALOG(14) | \
    PIN_MODE_INPUT(15))

#define VAL_GPIOC_OTYPER ( \
    PIN_OTYPE_PUSHPULL(0) | \
    PIN_OTYPE_PUSHPULL(1) | \
    PIN_OTYPE_PUSHPULL(2) | \
    PIN_OTYPE_PUSHPULL(3) | \
    PIN_OTYPE_PUSHPULL(4) | \
    PIN_OTYPE_PUSHPULL(5) | \
    PIN_OTYPE_PUSHPULL(6) | \
    PIN_OTYPE_PUSHPULL(7) | \
    PIN_OTYPE_OPENDRAIN(8) | \
    PIN_OTYPE_PUSHPULL(9) | \
    PIN_OTYPE_OPENDRAIN(10) | \
    PIN_OTYPE_OPENDRAIN(11) | \
    PIN_OTYPE_PUSHPULL(12) | \
    PIN_OTYPE_PUSHPULL(13) | \
    PIN_OTYPE_PUSHPULL(14) | \
    PIN_OTYPE_PUSHPULL(15))

#define VAL_GPIOC_OSPEEDR ( \
    PIN_OSPEED_2M(0) | \
    PIN_OSPEED_2M(1) | \
    PIN_OSPEED_2M(2) | \
    PIN_OSPEED_2M(3) | \
    PIN_OSPEED_2M(4) | \
    PIN_OSPEED_2M(5) | \
    PIN_OSPEED_50M(6) | \
    PIN_OSPEED_50M(7) | \
    PIN_OSPEED_2M(8) | \
    PIN_OSPEED_2M(9) | \
    PIN_OSPEED_50M(10) | \
    PIN_OSPEED_50M(11) | \
    PIN_OSPEED_2M(12) | \
    PIN_OSPEED_2M(13) | \
    PIN_OSPEED_2M(14) | \
    PIN_OSPEED_2M(15))

#define VAL_GPIOC_PUPDR ( \
    PIN_PUPDR_FLOATING(0) | \
    PIN_PUPDR_FLOATING(1) | \
    PIN_PUPDR_FLOATING(2) | \
    PIN_PUPDR_FLOATING(3) | \
    PIN_PUPDR_FLOATING(4) | \
    PIN_PUPDR_FLOATING(5) | \
    PIN_PUPDR_FLOATING(6) | \
    PIN_PUPDR_FLOATING(7) | \
    PIN_PUPDR_FLOATING(8) | \
    PIN_PUPDR_FLOATING(9) | \
    PIN_PUPDR_FLOATING(10) | \
    PIN_PUPDR_FLOATING(11) | \
    PIN_PUPDR_FLOATING(12) | \
    PIN_PUPDR_FLOATING(13) | \
    PIN_PUPDR_FLOATING(14) | \
    PIN_PUPDR_PULLUP(15))

#define VAL_GPIOC_ODR ( \
    PIN_ODR_HIGH(0) | \
    PIN_ODR_HIGH(1) | \
    PIN_ODR_HIGH(2) | \
    PIN_ODR_HIGH(3) | \
    PIN_ODR_HIGH(4) | \
    PIN_ODR_HIGH(5) | \
    PIN_ODR_HIGH(6) | \
    PIN_ODR_HIGH(7) | \
    PIN_ODR_HIGH(8) | \
    PIN_ODR_HIGH(9) | \
    PIN_ODR_HIGH(10) | \
    PIN_ODR_HIGH(11) | \
    PIN_ODR_HIGH(12) | \
    PIN_ODR_HIGH(13) | \
    PIN_ODR_HIGH(14) | \
    PIN_ODR_HIGH(15))

#define VAL_GPIOC_AFRL ( \
    PIN_AFIO_AF(0, 0) | \
    PIN_AFIO_AF(1, 0) | \
    PIN_AFIO_AF(2, 0) | \
    PIN_AFIO_AF(3, 0) | \
    PIN_AFIO_AF(4, 0) | \
    PIN_AFIO_AF(5, 0) | \
    PIN_AFIO_AF(6, 2) | \
    PIN_AFIO_AF(7, 2))

#define VAL_GPIOC_AFRH ( \
    PIN_AFIO_AF(8, 0) | \
    PIN_AFIO_AF(9, 0) | \
    PIN_AFIO_AF(10, 7) | \
    PIN_AFIO_AF(11, 7) | \
    PIN_AFIO_AF(12, 0) | \
    PIN_AFIO_AF(13, 0) | \
    PIN_AFIO_AF(14, 0) | \
    PIN_AFIO_AF(15, 0))

/* PORT D */
#define VAL_GPIOD_MODER ( \
    PIN_MODE_ANALOG(0) | \
    PIN_MODE_ANALOG(1) | \
    PIN_MODE_ANALOG(2) | \
    PIN_MODE_ANALOG(3) | \
    PIN_MODE_ANALOG(4) | \
    PIN_MODE_ANALOG(5) | \
    PIN_MODE_ANALOG(6) | \
    PIN_MODE_ANALOG(7) | \
    PIN_MODE_ANALOG(8) | \
    PIN_MODE_ANALOG(9) | \
    PIN_MODE_ANALOG(10) | \
    PIN_MODE_ANALOG(11) | \
    PIN_MODE_ANALOG(12) | \
    PIN_MODE_ANALOG(13) | \
    PIN_MODE_ANALOG(14) | \
    PIN_MODE_ANALOG(15))

#define VAL_GPIOD_OTYPER ( \
    PIN_OTYPE_PUSHPULL(0) | \
    PIN_OTYPE_PUSHPULL(1) | \
    PIN_OTYPE_PUSHPULL(2) | \
    PIN_OTYPE_PUSHPULL(3) | \
    PIN_OTYPE_PUSHPULL(4) | \
    PIN_OTYPE_PUSHPULL(5) | \
    PIN_OTYPE_PUSHPULL(6) | \
    PIN_OTYPE_PUSHPULL(7) | \
    PIN_OTYPE_PUSHPULL(8) | \
    PIN_OTYPE_PUSHPULL(9) | \
    PIN_OTYPE_PUSHPULL(10) | \
    PIN_OTYPE_PUSHPULL(11) | \
    PIN_OTYPE_PUSHPULL(12) | \
    PIN_OTYPE_PUSHPULL(13) | \
    PIN_OTYPE_PUSHPULL(14) | \
    PIN_OTYPE_PUSHPULL(15))

#define VAL_GPIOD_OSPEEDR ( \
    PIN_OSPEED_2M(0) | \
    PIN_OSPEED_2M(1) | \
    PIN_OSPEED_2M(2) | \
    PIN_OSPEED_2M(3) | \
    PIN_OSPEED_2M(4) | \
    PIN_OSPEED_2M(5) | \
    PIN_OSPEED_2M(6) | \
    PIN_OSPEED_2M(7) | \
    PIN_OSPEED_2M(8) | \
    PIN_OSPEED_2M(9) | \
    PIN_OSPEED_2M(10) | \
    PIN_OSPEED_2M(11) | \
    PIN_OSPEED_2M(12) | \
    PIN_OSPEED_2M(13) | \
    PIN_OSPEED_2M(14) | \
    PIN_OSPEED_2M(15))

#define VAL_GPIOD_PUPDR ( \
    PIN_PUPDR_FLOATING(0) | \
    PIN_PUPDR_FLOATING(1) | \
    PIN_PUPDR_FLOATING(2) | \
    PIN_PUPDR_FLOATING(3) | \
    PIN_PUPDR_FLOATING(4) | \
    PIN_PUPDR_FLOATING(5) | \
    PIN_PUPDR_FLOATING(6) | \
    PIN_PUPDR_FLOATING(7) | \
    PIN_PUPDR_FLOATING(8) | \
    PIN_PUPDR_FLOATING(9) | \
    PIN_PUPDR_FLOATING(10) | \
    PIN_PUPDR_FLOATING(11) | \
    PIN_PUPDR_FLOATING(12) | \
    PIN_PUPDR_FLOATING(13) | \
    PIN_PUPDR_FLOATING(14) | \
    PIN_PUPDR_FLOATING(15))

#define VAL_GPIOD_ODR ( \
    PIN_ODR_HIGH(0) | \
    PIN_ODR_HIGH(1) | \
    PIN_ODR_HIGH(2) | \
    PIN_ODR_HIGH(3) | \
    PIN_ODR_HIGH(4) | \
    PIN_ODR_HIGH(5) | \
    PIN_ODR_HIGH(6) | \
    PIN_ODR_HIGH(7) | \
    PIN_ODR_HIGH(8) | \
    PIN_ODR_HIGH(9) | \
    PIN_ODR_HIGH(10) | \
    PIN_ODR_HIGH(11) | \
    PIN_ODR_HIGH(12) | \
    PIN_ODR_HIGH(13) | \
    PIN_ODR_HIGH(14) | \
    PIN_ODR_HIGH(15))

#define VAL_GPIOD_AFRL ( \
    PIN_AFIO_AF(0, 0) | \
    PIN_AFIO_AF(1, 0) | \
    PIN_AFIO_AF(2, 0) | \
    PIN_AFIO_AF(3, 0) | \
    PIN_AFIO_AF(4, 0) | \
    PIN_AFIO_AF(5, 0) | \
    PIN_AFIO_AF(6, 0) | \
    PIN_AFIO_AF(7, 0))

#define VAL_GPIOD_AFRH ( \
    PIN_AFIO_AF(8, 0) | \
    PIN_AFIO_AF(9, 0) | \
    PIN_AFIO_AF(10, 0) | \
    PIN_AFIO_AF(11, 0) | \
    PIN_AFIO_AF(12, 0) | \
    PIN_AFIO_AF(13, 0) | \
    PIN_AFIO_AF(14, 0) | \
    PIN_AFIO_AF(15, 0))

/* PORT E */
#define VAL_GPIOE_MODER ( \
    PIN_MODE_ANALOG(0) | \
    PIN_MODE_ANALOG(1) | \
    PIN_MODE_ANALOG(2) | \
    PIN_MODE_ANALOG(3) | \
    PIN_MODE_ANALOG(4) | \
    PIN_MODE_ANALOG(5) | \
    PIN_MODE_ANALOG(6) | \
    PIN_MODE_ANALOG(7) | \
    PIN_MODE_ANALOG(8) | \
    PIN_MODE_ANALOG(9) | \
    PIN_MODE_ANALOG(10) | \
    PIN_MODE_ANALOG(11) | \
    PIN_MODE_ANALOG(12) | \
    PIN_MODE_ANALOG(13) | \
    PIN_MODE_ANALOG(14) | \
    PIN_MODE_ANALOG(15))

#define VAL_GPIOE_OTYPER ( \
    PIN_OTYPE_PUSHPULL(0) | \
    PIN_OTYPE_PUSHPULL(1) | \
    PIN_OTYPE_PUSHPULL(2) | \
    PIN_OTYPE_PUSHPULL(3) | \
    PIN_OTYPE_PUSHPULL(4) | \
    PIN_OTYPE_PUSHPULL(5) | \
    PIN_OTYPE_PUSHPULL(6) | \
    PIN_OTYPE_PUSHPULL(7) | \
    PIN_OTYPE_PUSHPULL(8) | \
    PIN_OTYPE_PUSHPULL(9) | \
    PIN_OTYPE_PUSHPULL(10) | \
    PIN_OTYPE_PUSHPULL(11) | \
    PIN_OTYPE_PUSHPULL(12) | \
    PIN_OTYPE_PUSHPULL(13) | \
    PIN_OTYPE_PUSHPULL(14) | \
    PIN_OTYPE_PUSHPULL(15))

#define VAL_GPIOE_OSPEEDR ( \
    PIN_OSPEED_2M(0) | \
    PIN_OSPEED_2M(1) | \
    PIN_OSPEED_2M(2) | \
    PIN_OSPEED_2M(3) | \
    PIN_OSPEED_2M(4) | \
    PIN_OSPEED_2M(5) | \
    PIN_OSPEED_2M(6) | \
    PIN_OSPEED_2M(7) | \
    PIN_OSPEED_2M(8) | \
    PIN_OSPEED_2M(9) | \
    PIN_OSPEED_2M(10) | \
    PIN_OSPEED_2M(11) | \
    PIN_OSPEED_2M(12) | \
    PIN_OSPEED_2M(13) | \
    PIN_OSPEED_2M(14) | \
    PIN_OSPEED_2M(15))

#define VAL_GPIOE_PUPDR ( \
    PIN_PUPDR_FLOATING(0) | \
    PIN_PUPDR_FLOATING(1) | \
    PIN_PUPDR_FLOATING(2) | \
    PIN_PUPDR_FLOATING(3) | \
    PIN_PUPDR_FLOATING(4) | \
    PIN_PUPDR_FLOATING(5) | \
    PIN_PUPDR_FLOATING(6) | \
    PIN_PUPDR_FLOATING(7) | \
    PIN_PUPDR_FLOATING(8) | \
    PIN_PUPDR_FLOATING(9) | \
    PIN_PUPDR_FLOATING(10) | \
    PIN_PUPDR_FLOATING(11) | \
    PIN_PUPDR_FLOATING(12) | \
    PIN_PUPDR_FLOATING(13) | \
    PIN_PUPDR_FLOATING(14) | \
    PIN_PUPDR_FLOATING(15))

#define VAL_GPIOE_ODR ( \
    PIN_ODR_HIGH(0) | \
    PIN_ODR_HIGH(1) | \
    PIN_ODR_HIGH(2) | \
    PIN_ODR_HIGH(3) | \
    PIN_ODR_HIGH(4) | \
    PIN_ODR_HIGH(5) | \
    PIN_ODR_HIGH(6) | \
    PIN_ODR_HIGH(7) | \
    PIN_ODR_HIGH(8) | \
    PIN_ODR_HIGH(9) | \
    PIN_ODR_HIGH(10) | \
    PIN_ODR_HIGH(11) | \
    PIN_ODR_HIGH(12) | \
    PIN_ODR_HIGH(13) | \
    PIN_ODR_HIGH(14) | \
    PIN_ODR_HIGH(15))

#define VAL_GPIOE_AFRL ( \
    PIN_AFIO_AF(0, 0) | \
    PIN_AFIO_AF(1, 0) | \
    PIN_AFIO_AF(2, 0) | \
    PIN_AFIO_AF(3, 0) | \
    PIN_AFIO_AF(4, 0) | \
    PIN_AFIO_AF(5, 0) | \
    PIN_AFIO_AF(6, 0) | \
    PIN_AFIO_AF(7, 0))

#define VAL_GPIOE_AFRH ( \
    PIN_AFIO_AF(8, 0) | \
    PIN_AFIO_AF(9, 0) | \
    PIN_AFIO_AF(10, 0) | \
    PIN_AFIO_AF(11, 0) | \
    PIN_AFIO_AF(12, 0) | \
    PIN_AFIO_AF(13, 0) | \
    PIN_AFIO_AF(14, 0) | \
    PIN_AFIO_AF(15, 0))

/* PORT F */
#define VAL_GPIOF_MODER ( \
    PIN_MODE_INPUT(0) | \
    PIN_MODE_INPUT(1) | \
    PIN_MODE_ANALOG(2) | \
    PIN_MODE_ANALOG(3) | \
    PIN_MODE_INPUT(4) | \
    PIN_MODE_ANALOG(5) | \
    PIN_MODE_ANALOG(6) | \
    PIN_MODE_ANALOG(7) | \
    PIN_MODE_ANALOG(8) | \
    PIN_MODE_ANALOG(9) | \
    PIN_MODE_ANALOG(10) | \
    PIN_MODE_ANALOG(11) | \
    PIN_MODE_ANALOG(12) | \
    PIN_MODE_ANALOG(13) | \
    PIN_MODE_ANALOG(14) | \
    PIN_MODE_ANALOG(15))

#define VAL_GPIOF_OTYPER ( \
    PIN_OTYPE_PUSHPULL(0) | \
    PIN_OTYPE_PUSHPULL(1) | \
    PIN_OTYPE_PUSHPULL(2) | \
    PIN_OTYPE_PUSHPULL(3) | \
    PIN_OTYPE_PUSHPULL(4) | \
    PIN_OTYPE_PUSHPULL(5) | \
    PIN_OTYPE_PUSHPULL(6) | \
    PIN_OTYPE_PUSHPULL(7) | \
    PIN_OTYPE_PUSHPULL(8) | \
    PIN_OTYPE_PUSHPULL(9) | \
    PIN_OTYPE_PUSHPULL(10) | \
    PIN_OTYPE_PUSHPULL(11) | \
    PIN_OTYPE_PUSHPULL(12) | \
    PIN_OTYPE_PUSHPULL(13) | \
    PIN_OTYPE_PUSHPULL(14) | \
    PIN_OTYPE_PUSHPULL(15))

#define VAL_GPIOF_OSPEEDR ( \
    PIN_OSPEED_2M(0) | \
    PIN_OSPEED_2M(1) | \
    PIN_OSPEED_2M(2) | \
    PIN_OSPEED_2M(3) | \
    PIN_OSPEED_2M(4) | \
    PIN_OSPEED_2M(5) | \
    PIN_OSPEED_2M(6) | \
    PIN_OSPEED_2M(7) | \
    PIN_OSPEED_2M(8) | \
    PIN_OSPEED_2M(9) | \
    PIN_OSPEED_2M(10) | \
    PIN_OSPEED_2M(11) | \
    PIN_OSPEED_2M(12) | \
    PIN_OSPEED_2M(13) | \
    PIN_OSPEED_2M(14) | \
    PIN_OSPEED_2M(15))

#define VAL_GPIOF_PUPDR ( \
    PIN_PUPDR_FLOATING(0) | \
    PIN_PUPDR_FLOATING(1) | \
    PIN_PUPDR_FLOATING(2) | \
    PIN_PUPDR_FLOATING(3) | \
    PIN_PUPDR_PULLUP(4) | \
    PIN_PUPDR_FLOATING(5) | \
    PIN_PUPDR_FLOATING(6) | \
    PIN_PUPDR_FLOATING(7) | \
    PIN_PUPDR_FLOATING(8) | \
    PIN_PUPDR_FLOATING(9) | \
    PIN_PUPDR_FLOATING(10) | \
    PIN_PUPDR_FLOATING(11) | \
    PIN_PUPDR_FLOATING(12) | \
    PIN_PUPDR_FLOATING(13) | \
    PIN_PUPDR_FLOATING(14) | \
    PIN_PUPDR_FLOATING(15))

#define VAL_GPIOF_ODR ( \
    PIN_ODR_HIGH(0) | \
    PIN_ODR_HIGH(1) | \
    PIN_ODR_HIGH(2) | \
    PIN_ODR_HIGH(3) | \
    PIN_ODR_HIGH(4) | \
    PIN_ODR_HIGH(5) | \
    PIN_ODR_HIGH(6) | \
    PIN_ODR_HIGH(7) | \
    PIN_ODR_HIGH(8) | \
    PIN_ODR_HIGH(9) | \
    PIN_ODR_HIGH(10) | \
    PIN_ODR_HIGH(11) | \
    PIN_ODR_HIGH(12) | \
    PIN_ODR_HIGH(13) | \
    PIN_ODR_HIGH(14) | \
    PIN_ODR_HIGH(15))

#define VAL_GPIOF_AFRL ( \
    PIN_AFIO_AF(0, 0) | \
    PIN_AFIO_AF(1, 0) | \
    PIN_AFIO_AF(2, 0) | \
    PIN_AFIO_AF(3, 0) | \
    PIN_AFIO_AF(4, 0) | \
    PIN_AFIO_AF(5, 0) | \
    PIN_AFIO_AF(6, 0) | \
    PIN_AFIO_AF(7, 0))

#define VAL_GPIOF_AFRH ( \
    PIN_AFIO_AF(8, 0) | \
    PIN_AFIO_AF(9, 0) | \
    PIN_AFIO_AF(10, 0) | \
    PIN_AFIO_AF(11, 0) | \
    PIN_AFIO_AF(12, 0) | \
    PIN_AFIO_AF(13, 0) | \
    PIN_AFIO_AF(14, 0) | \
    PIN_AFIO_AF(15, 0))

#endif
