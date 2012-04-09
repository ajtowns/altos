/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _STM32L_H_
#define _STM32L_H_

#include <stdint.h>

typedef volatile uint32_t	vuint32_t;
typedef volatile void *		vvoid_t;

struct stm_gpio {
	vuint32_t	moder;
	vuint32_t	otyper;
	vuint32_t	ospeedr;
	vuint32_t	pupdr;

	vuint32_t	idr;
	vuint32_t	odr;
	vuint32_t	bsrr;
	vuint32_t	lckr;

	vuint32_t	afrl;
	vuint32_t	afrh;
};

#define STM_MODER_SHIFT(pin)		((pin) << 1)
#define STM_MODER_MASK			3
#define STM_MODER_INPUT			0
#define STM_MODER_OUTPUT		1
#define STM_MODER_ALTERNATE		2
#define STM_MODER_ANALOG		3

static inline void
stm_moder_set(struct stm_gpio *gpio, int pin, vuint32_t value) {
	gpio->moder = ((gpio->moder &
			~(STM_MODER_MASK << STM_MODER_SHIFT(pin))) |
		       value << STM_MODER_SHIFT(pin));
}
	
static inline vuint32_t
stm_moder_get(struct stm_gpio *gpio, int pin) {
	return (gpio->moder >> STM_MODER_SHIFT(pin)) & STM_MODER_MASK;
}

#define STM_OTYPER_SHIFT(pin)		(pin)
#define STM_OTYPER_MASK			1
#define STM_OTYPER_PUSH_PULL		0
#define STM_OTYPER_OPEN_DRAIN		1

static inline void
stm_otyper_set(struct stm_gpio *gpio, int pin, vuint32_t value) {
	gpio->otyper = ((gpio->otyper &
			 ~(STM_OTYPER_MASK << STM_OTYPER_SHIFT(pin))) |
			value << STM_OTYPER_SHIFT(pin));
}
	
static inline vuint32_t
stm_otyper_get(struct stm_gpio *gpio, int pin) {
	return (gpio->otyper >> STM_OTYPER_SHIFT(pin)) & STM_OTYPER_MASK;
}

#define STM_OSPEEDR_SHIFT(pin)		((pin) << 1)
#define STM_OSPEEDR_MASK		3
#define STM_OSPEEDR_400kHz		0
#define STM_OSPEEDR_2MHz		1
#define STM_OSPEEDR_10MHz		2
#define STM_OSPEEDR_40MHz		3

static inline void
stm_ospeedr_set(struct stm_gpio *gpio, int pin, vuint32_t value) {
	gpio->ospeedr = ((gpio->ospeedr &
			~(STM_OSPEEDR_MASK << STM_OSPEEDR_SHIFT(pin))) |
		       value << STM_OSPEEDR_SHIFT(pin));
}
	
static inline vuint32_t
stm_ospeedr_get(struct stm_gpio *gpio, int pin) {
	return (gpio->ospeedr >> STM_OSPEEDR_SHIFT(pin)) & STM_OSPEEDR_MASK;
}

#define STM_PUPDR_SHIFT(pin)		((pin) << 1)
#define STM_PUPDR_MASK			3
#define STM_PUPDR_NONE			0
#define STM_PUPDR_PULL_UP		1
#define STM_PUPDR_PULL_DOWN		2
#define STM_PUPDR_RESERVED		3

static inline void
stm_pupdr_set(struct stm_gpio *gpio, int pin, vuint32_t value) {
	gpio->pupdr = ((gpio->pupdr &
			~(STM_PUPDR_MASK << STM_PUPDR_SHIFT(pin))) |
		       value << STM_PUPDR_SHIFT(pin));
}
	
static inline vuint32_t
stm_pupdr_get(struct stm_gpio *gpio, int pin) {
	return (gpio->pupdr >> STM_PUPDR_SHIFT(pin)) & STM_PUPDR_MASK;
}

#define STM_AFR_SHIFT(pin)		((pin) << 2)
#define STM_AFR_MASK			0xf
#define STM_AFR_NONE			0
#define STM_AFR_AF0			0x0
#define STM_AFR_AF1			0x1
#define STM_AFR_AF2			0x2
#define STM_AFR_AF3			0x3
#define STM_AFR_AF4			0x4
#define STM_AFR_AF5			0x5
#define STM_AFR_AF6			0x6
#define STM_AFR_AF7			0x7
#define STM_AFR_AF8			0x8
#define STM_AFR_AF9			0x9
#define STM_AFR_AF10			0xa
#define STM_AFR_AF11			0xb
#define STM_AFR_AF12			0xc
#define STM_AFR_AF13			0xd
#define STM_AFR_AF14			0xe
#define STM_AFR_AF15			0xf

static inline void
stm_afr_set(struct stm_gpio *gpio, int pin, uint32_t value) {
	/*
	 * Set alternate pin mode too
	 */
	stm_moder_set(gpio, pin, STM_MODER_ALTERNATE);
	if (pin < 8)
		gpio->afrl = ((gpio->afrl &
			       ~(STM_AFR_MASK << STM_AFR_SHIFT(pin))) |
			      value << STM_AFR_SHIFT(pin));
	else {
		pin -= 8;
		gpio->afrh = ((gpio->afrh &
			       ~(STM_AFR_MASK << STM_AFR_SHIFT(pin))) |
			      value << STM_AFR_SHIFT(pin));
	}
}
	
static inline uint32_t
stm_afr_get(struct stm_gpio *gpio, int pin) {
	if (pin < 8)
		return (gpio->afrl >> STM_AFR_SHIFT(pin)) & STM_AFR_MASK;
	else {
		pin -= 8;
		return (gpio->afrh >> STM_AFR_SHIFT(pin)) & STM_AFR_MASK;
	}
}

static inline void
stm_gpio_set(struct stm_gpio *gpio, int pin, uint8_t value) {
	/* Use the bit set/reset register to do this atomically */
	gpio->bsrr = ((uint32_t) (value ^ 1) << (pin + 16)) | ((uint32_t) value << pin);
}

static inline uint8_t
stm_gpio_isset(struct stm_gpio *gpio, int pin) {
	return (gpio->idr >> pin) & 1;
}

extern struct stm_gpio stm_gpioa;
extern struct stm_gpio stm_gpiob;
extern struct stm_gpio stm_gpioc;
extern struct stm_gpio stm_gpiod;
extern struct stm_gpio stm_gpioe;
extern struct stm_gpio stm_gpioh;

struct stm_usart {
	vuint32_t	sr;	/* status register */
	vuint32_t	dr;	/* data register */
	vuint32_t	brr;	/* baud rate register */
	vuint32_t	cr1;	/* control register 1 */

	vuint32_t	cr2;	/* control register 2 */
	vuint32_t	cr3;	/* control register 3 */
	vuint32_t	gtpr;	/* guard time and prescaler */
};

extern struct stm_usart	stm_usart1;
extern struct stm_usart stm_usart2;
extern struct stm_usart stm_usart3;

#define STM_USART_SR_CTS	(9)	/* CTS flag */
#define STM_USART_SR_LBD	(8)	/* LIN break detection flag */
#define STM_USART_SR_TXE	(7)	/* Transmit data register empty */
#define STM_USART_SR_TC		(6)	/* Transmission complete */
#define STM_USART_SR_RXNE	(5)	/* Read data register not empty */
#define STM_USART_SR_IDLE	(4)	/* IDLE line detected */
#define STM_USART_SR_ORE	(3)	/* Overrun error */
#define STM_USART_SR_NF		(2)	/* Noise detected flag */
#define STM_USART_SR_FE		(1)	/* Framing error */
#define STM_USART_SR_PE		(0)	/* Parity error */

#define STM_USART_CR1_OVER8	(15)	/* Oversampling mode */
#define STM_USART_CR1_UE	(13)	/* USART enable */
#define STM_USART_CR1_M		(12)	/* Word length */
#define STM_USART_CR1_WAKE	(11)	/* Wakeup method */
#define STM_USART_CR1_PCE	(10)	/* Parity control enable */
#define STM_USART_CR1_PS	(9)	/* Parity selection */
#define STM_USART_CR1_PEIE	(8)	/* PE interrupt enable */
#define STM_USART_CR1_TXEIE	(7)	/* TXE interrupt enable */
#define STM_USART_CR1_TCIE	(6)	/* Transmission complete interrupt enable */
#define STM_USART_CR1_RXNEIE	(5)	/* RXNE interrupt enable */
#define STM_USART_CR1_IDLEIE	(4)	/* IDLE interrupt enable */
#define STM_USART_CR1_TE	(3)	/* Transmitter enable */
#define STM_USART_CR1_RE	(2)	/* Receiver enable */
#define STM_USART_CR1_RWU	(1)	/* Receiver wakeup */
#define STM_USART_CR1_SBK	(0)	/* Send break */

#define STM_USART_CR2_LINEN	(14)	/* LIN mode enable */
#define STM_USART_CR2_STOP	(12)	/* STOP bits */
#define STM_USART_CR2_STOP_MASK	3
#define STM_USART_CR2_STOP_1	0
#define STM_USART_CR2_STOP_0_5	1
#define STM_USART_CR2_STOP_2	2
#define STM_USART_CR2_STOP_1_5	3

#define STM_USART_CR2_CLKEN	(11)	/* Clock enable */
#define STM_USART_CR2_CPOL	(10)	/* Clock polarity */
#define STM_USART_CR2_CPHA	(9)	/* Clock phase */
#define STM_USART_CR2_LBCL	(8)	/* Last bit clock pulse */
#define STM_USART_CR2_LBDIE	(6)	/* LIN break detection interrupt enable */
#define STM_USART_CR2_LBDL	(5)	/* lin break detection length */
#define STM_USART_CR2_ADD	(0)
#define STM_USART_CR2_ADD_MASK	0xf

#define STM_USART_CR3_ONEBITE	(11)	/* One sample bit method enable */
#define STM_USART_CR3_CTSIE	(10)	/* CTS interrupt enable */
#define STM_USART_CR3_CTSE	(9)	/* CTS enable */
#define STM_USART_CR3_RTSE	(8)	/* RTS enable */
#define STM_USART_CR3_DMAT	(7)	/* DMA enable transmitter */
#define STM_USART_CR3_DMAR	(6)	/* DMA enable receiver */
#define STM_USART_CR3_SCEN	(5)	/* Smartcard mode enable */
#define STM_USART_CR3_NACK	(4)	/* Smartcard NACK enable */
#define STM_USART_CR3_HDSEL	(3)	/* Half-duplex selection */
#define STM_USART_CR3_IRLP	(2)	/* IrDA low-power */
#define STM_USART_CR3_IREN	(1)	/* IrDA mode enable */
#define STM_USART_CR3_EIE	(0)	/* Error interrupt enable */

struct stm_tim {
};

extern struct stm_tim stm_tim9;
extern struct stm_tim stm_tim10;
extern struct stm_tim stm_tim11;

/* Flash interface */

struct stm_flash {
	vuint32_t	acr;
	vuint32_t	pecr;
	vuint32_t	pdkeyr;
	vuint32_t	pekeyr;

	vuint32_t	prgkeyr;
	vuint32_t	optkeyr;
	vuint32_t	sr;
	vuint32_t	obr;

	vuint32_t	wrpr;
};

extern struct stm_flash	stm_flash;

#define STM_FLASH_ACR_RUN_PD	(4)
#define STM_FLASH_ACR_SLEEP_PD	(3)
#define STM_FLASH_ACR_ACC64	(2)
#define STM_FLASH_ACR_PRFEN	(1)
#define STM_FLASH_ACR_LATENCY	(0)

struct stm_rcc {
	vuint32_t	cr;
	vuint32_t	icscr;
	vuint32_t	cfgr;
	vuint32_t	cir;

	vuint32_t	ahbrstr;
	vuint32_t	apb2rstr;
	vuint32_t	abp1rstr;
	vuint32_t	ahbenr;

	vuint32_t	apb2enr;
	vuint32_t	apb1enr;
	vuint32_t	ahblenr;
	vuint32_t	apb2lpenr;

	vuint32_t	apb1lpenr;
	vuint32_t	csr;
};

extern struct stm_rcc stm_rcc;

#define STM_RCC_CR_RTCPRE	(29)
#define  STM_RCC_CR_RTCPRE_HSE_DIV_2	0
#define  STM_RCC_CR_RTCPRE_HSE_DIV_4	1
#define  STM_RCC_CR_RTCPRE_HSE_DIV_8	2
#define  STM_RCC_CR_RTCPRE_HSE_DIV_16	3
#define  STM_RCC_CR_RTCPRE_HSE_MASK	3

#define STM_RCC_CR_CSSON	(28)
#define STM_RCC_CR_PLLRDY	(25)
#define STM_RCC_CR_PLLON	(24)
#define STM_RCC_CR_HSEBYP	(18)
#define STM_RCC_CR_HSERDY	(17)
#define STM_RCC_CR_HSEON	(16)
#define STM_RCC_CR_MSIRDY	(9)
#define STM_RCC_CR_MSION	(8)
#define STM_RCC_CR_HSIRDY	(1)
#define STM_RCC_CR_HSION	(0)

#define STM_RCC_CFGR_MCOPRE	(28)
#define  STM_RCC_CFGR_MCOPRE_DIV_1	0
#define  STM_RCC_CFGR_MCOPRE_DIV_2	1
#define  STM_RCC_CFGR_MCOPRE_DIV_4	2
#define  STM_RCC_CFGR_MCOPRE_DIV_8	3
#define  STM_RCC_CFGR_MCOPRE_DIV_16	4
#define  STM_RCC_CFGR_MCOPRE_DIV_MASK	7

#define STM_RCC_CFGR_MCOSEL	(24)
#define  STM_RCC_CFGR_MCOSEL_DISABLE	0
#define  STM_RCC_CFGR_MCOSEL_SYSCLK	1
#define  STM_RCC_CFGR_MCOSEL_HSI	2
#define  STM_RCC_CFGR_MCOSEL_MSI	3
#define  STM_RCC_CFGR_MCOSEL_HSE	4
#define  STM_RCC_CFGR_MCOSEL_PLL	5
#define  STM_RCC_CFGR_MCOSEL_LSI	6
#define  STM_RCC_CFGR_MCOSEL_LSE	7
#define  STM_RCC_CFGR_MCOSEL_MASK	7

#define STM_RCC_CFGR_PLLDIV	(22)
#define  STM_RCC_CFGR_PLLDIV_2		1
#define  STM_RCC_CFGR_PLLDIV_3		2
#define  STM_RCC_CFGR_PLLDIV_4		3
#define  STM_RCC_CFGR_PLLDIV_MASK	3

#define STM_RCC_CFGR_PLLMUL	(18)
#define  STM_RCC_CFGR_PLLMUL_3		0
#define  STM_RCC_CFGR_PLLMUL_4		1
#define  STM_RCC_CFGR_PLLMUL_6		2
#define  STM_RCC_CFGR_PLLMUL_8		3
#define  STM_RCC_CFGR_PLLMUL_12		4
#define  STM_RCC_CFGR_PLLMUL_16		5
#define  STM_RCC_CFGR_PLLMUL_24		6
#define  STM_RCC_CFGR_PLLMUL_32		7
#define  STM_RCC_CFGR_PLLMUL_48		8
#define  STM_RCC_CFGR_PLLMUL_MASK	0xf

#define STM_RCC_CFGR_PLLSRC	(16)

#define STM_RCC_CFGR_PPRE2	(11)
#define  STM_RCC_CFGR_PPRE2_DIV_1	0
#define  STM_RCC_CFGR_PPRE2_DIV_2	4
#define  STM_RCC_CFGR_PPRE2_DIV_4	5
#define  STM_RCC_CFGR_PPRE2_DIV_8	6
#define  STM_RCC_CFGR_PPRE2_DIV_16	7
#define  STM_RCC_CFGR_PPRE2_MASK	7

#define STM_RCC_CFGR_PPRE1	(8)
#define  STM_RCC_CFGR_PPRE1_DIV_1	0
#define  STM_RCC_CFGR_PPRE1_DIV_2	4
#define  STM_RCC_CFGR_PPRE1_DIV_4	5
#define  STM_RCC_CFGR_PPRE1_DIV_8	6
#define  STM_RCC_CFGR_PPRE1_DIV_16	7
#define  STM_RCC_CFGR_PPRE1_MASK	7

#define STM_RCC_CFGR_HPRE	(4)
#define  STM_RCC_CFGR_HPRE_DIV_1	0
#define  STM_RCC_CFGR_HPRE_DIV_2	8
#define  STM_RCC_CFGR_HPRE_DIV_4	9
#define  STM_RCC_CFGR_HPRE_DIV_8	0xa
#define  STM_RCC_CFGR_HPRE_DIV_16	0xb
#define  STM_RCC_CFGR_HPRE_DIV_64	0xc
#define  STM_RCC_CFGR_HPRE_DIV_128	0xd
#define  STM_RCC_CFGR_HPRE_DIV_256	0xe
#define  STM_RCC_CFGR_HPRE_DIV_512	0xf
#define  STM_RCC_CFGR_HPRE_MASK		0xf

#define STM_RCC_CFGR_SWS	(2)
#define  STM_RCC_CFGR_SWS_MSI		0
#define  STM_RCC_CFGR_SWS_HSI		1
#define  STM_RCC_CFGR_SWS_HSE		2
#define  STM_RCC_CFGR_SWS_PLL		3
#define  STM_RCC_CFGR_SWS_MASK		3

#define STM_RCC_CFGR_SW		(0)
#define  STM_RCC_CFGR_SW_MSI		0
#define  STM_RCC_CFGR_SW_HSI		1
#define  STM_RCC_CFGR_SW_HSE		2
#define  STM_RCC_CFGR_SW_PLL		3
#define  STM_RCC_CFGR_SW_MASK		3

#define STM_RCC_AHBENR_DMA1EN		(24)
#define STM_RCC_AHBENR_FLITFEN		(15)
#define STM_RCC_AHBENR_CRCEN		(12)
#define STM_RCC_AHBENR_GPIOHEN		(5)
#define STM_RCC_AHBENR_GPIOEEN		(4)
#define STM_RCC_AHBENR_GPIODEN		(3)
#define STM_RCC_AHBENR_GPIOCEN		(2)
#define STM_RCC_AHBENR_GPIOBEN		(1)
#define STM_RCC_AHBENR_GPIOAEN		(0)

#define STM_RCC_APB2ENR_USART1EN	(14)
#define STM_RCC_APB2ENR_SPI1EN		(12)
#define STM_RCC_APB2ENR_ADC1EN		(9)
#define STM_RCC_APB2ENR_TIM11EN		(4)
#define STM_RCC_APB2ENR_TIM10EN		(3)
#define STM_RCC_APB2ENR_TIM9EN		(2)
#define STM_RCC_APB2ENR_SYSCFGEN	(0)

#define STM_RCC_APB1ENR_COMPEN		(31)
#define STM_RCC_APB1ENR_DACEN		(29)
#define STM_RCC_APB1ENR_PWREN		(28)
#define STM_RCC_APB1ENR_USBEN		(23)
#define STM_RCC_APB1ENR_I2C2EN		(22)
#define STM_RCC_APB1ENR_I2C1EN		(21)
#define STM_RCC_APB1ENR_USART3EN	(18)
#define STM_RCC_APB1ENR_USART2EN	(17)
#define STM_RCC_APB1ENR_SPI2EN		(14)
#define STM_RCC_APB1ENR_WWDGEN		(11)
#define STM_RCC_APB1ENR_LCDEN		(9)
#define STM_RCC_APB1ENR_TIM7EN		(5)
#define STM_RCC_APB1ENR_TIM6EN		(4)
#define STM_RCC_APB1ENR_TIM4EN		(2)
#define STM_RCC_APB1ENR_TIM3EN		(1)
#define STM_RCC_APB1ENR_TIM2EN		(0)

#define STM_RCC_CSR_LPWRRSTF		(31)
#define STM_RCC_CSR_WWDGRSTF		(30)
#define STM_RCC_CSR_IWDGRSTF		(29)
#define STM_RCC_CSR_SFTRSTF		(28)
#define STM_RCC_CSR_PORRSTF		(27)
#define STM_RCC_CSR_PINRSTF		(26)
#define STM_RCC_CSR_OBLRSTF		(25)
#define STM_RCC_CSR_RMVF		(24)
#define STM_RCC_CSR_RTFRST		(23)
#define STM_RCC_CSR_RTCEN		(22)
#define STM_RCC_CSR_RTCSEL		(16)

#define  STM_RCC_CSR_RTCSEL_NONE		0
#define  STM_RCC_CSR_RTCSEL_LSE			1
#define  STM_RCC_CSR_RTCSEL_LSI			2
#define  STM_RCC_CSR_RTCSEL_HSE			3
#define  STM_RCC_CSR_RTCSEL_MASK		3

#define STM_RCC_CSR_LSEBYP		(10)
#define STM_RCC_CSR_LSERDY		(9)
#define STM_RCC_CSR_LSEON		(8)
#define STM_RCC_CSR_LSIRDY		(1)
#define STM_RCC_CSR_LSION		(0)

struct stm_pwr {
	vuint32_t	cr;
	vuint32_t	csr;
};

extern struct stm_pwr stm_pwr;

#define STM_PWR_CR_LPRUN	(14)

#define STM_PWR_CR_VOS		(11)
#define  STM_PWR_CR_VOS_1_8		1
#define  STM_PWR_CR_VOS_1_5		2
#define  STM_PWR_CR_VOS_1_2		3
#define  STM_PWR_CR_VOS_MASK		3

#define STM_PWR_CR_FWU		(10)
#define STM_PWR_CR_ULP		(9)
#define STM_PWR_CR_DBP		(8)

#define STM_PWR_CR_PLS		(5)
#define  STM_PWR_CR_PLS_1_9	0
#define  STM_PWR_CR_PLS_2_1	1
#define  STM_PWR_CR_PLS_2_3	2
#define  STM_PWR_CR_PLS_2_5	3
#define  STM_PWR_CR_PLS_2_7	4
#define  STM_PWR_CR_PLS_2_9	5
#define  STM_PWR_CR_PLS_3_1	6
#define  STM_PWR_CR_PLS_EXT	7
#define  STM_PWR_CR_PLS_MASK	7

#define STM_PWR_CR_PVDE		(4)
#define STM_PWR_CR_CSBF		(3)
#define STM_PWR_CR_CWUF		(2)
#define STM_PWR_CR_PDDS		(1)
#define STM_PWR_CR_LPSDSR	(0)

#define STM_PWR_CSR_EWUP3	(10)
#define STM_PWR_CSR_EWUP2	(9)
#define STM_PWR_CSR_EWUP1	(8)
#define STM_PWR_CSR_REGLPF	(5)
#define STM_PWR_CSR_VOSF	(4)
#define STM_PWR_CSR_VREFINTRDYF	(3)
#define STM_PWR_CSR_PVDO	(2)
#define STM_PWR_CSR_SBF		(1)
#define STM_PWR_CSR_WUF		(0)

struct stm_tim67 {
	vuint32_t	cr1;
	vuint32_t	cr2;
	uint32_t	_unused_08;
	vuint32_t	dier;

	vuint32_t	sr;
	vuint32_t	egr;
	uint32_t	_unused_18;
	uint32_t	_unused_1c;

	uint32_t	_unused_20;
	vuint32_t	cnt;
	vuint32_t	psc;
	vuint32_t	arr;
};

extern struct stm_tim67 stm_tim6;

#define STM_TIM67_CR1_ARPE	(7)
#define STM_TIM67_CR1_OPM	(3)
#define STM_TIM67_CR1_URS	(2)
#define STM_TIM67_CR1_UDIS	(1)
#define STM_TIM67_CR1_CEN	(0)

#define STM_TIM67_CR2_MMS	(4)
#define  STM_TIM67_CR2_MMS_RESET	0
#define  STM_TIM67_CR2_MMS_ENABLE	1
#define  STM_TIM67_CR2_MMS_UPDATE	2
#define  STM_TIM67_CR2_MMS_MASK		7

#define STM_TIM67_DIER_UDE	(8)
#define STM_TIM67_DIER_UIE	(0)

#define STM_TIM67_SR_UIF	(0)

#define STM_TIM67_EGR_UG	(0)

struct stm_lcd {
	vuint32_t	cr;
	vuint32_t	fcr;
	vuint32_t	sr;
	vuint32_t	clr;
	uint32_t	unused_0x10;
	vuint32_t	ram[8*2];
};

extern struct stm_lcd stm_lcd;

#define STM_LCD_CR_MUX_SEG		(7)

#define STM_LCD_CR_BIAS			(5)
#define  STM_LCD_CR_BIAS_1_4		0
#define  STM_LCD_CR_BIAS_1_2		1
#define  STM_LCD_CR_BIAS_1_3		2
#define  STM_LCD_CR_BIAS_MASK		3

#define STM_LCD_CR_DUTY			(2)
#define  STM_LCD_CR_DUTY_STATIC		0
#define  STM_LCD_CR_DUTY_1_2		1
#define  STM_LCD_CR_DUTY_1_3		2
#define  STM_LCD_CR_DUTY_1_4		3
#define  STM_LCD_CR_DUTY_1_8		4
#define  STM_LCD_CR_DUTY_MASK		7

#define STM_LCD_CR_VSEL			(1)
#define STM_LCD_CR_LCDEN		(0)

#define STM_LCD_FCR_PS			(22)
#define  STM_LCD_FCR_PS_1		0x0
#define  STM_LCD_FCR_PS_2		0x1
#define  STM_LCD_FCR_PS_4		0x2
#define  STM_LCD_FCR_PS_8		0x3
#define  STM_LCD_FCR_PS_16		0x4
#define  STM_LCD_FCR_PS_32		0x5
#define  STM_LCD_FCR_PS_64		0x6
#define  STM_LCD_FCR_PS_128		0x7
#define  STM_LCD_FCR_PS_256		0x8
#define  STM_LCD_FCR_PS_512		0x9
#define  STM_LCD_FCR_PS_1024		0xa
#define  STM_LCD_FCR_PS_2048		0xb
#define  STM_LCD_FCR_PS_4096		0xc
#define  STM_LCD_FCR_PS_8192		0xd
#define  STM_LCD_FCR_PS_16384		0xe
#define  STM_LCD_FCR_PS_32768		0xf
#define  STM_LCD_FCR_PS_MASK		0xf

#define STM_LCD_FCR_DIV			(18)
#define STM_LCD_FCR_DIV_16		0x0
#define STM_LCD_FCR_DIV_17		0x1
#define STM_LCD_FCR_DIV_18		0x2
#define STM_LCD_FCR_DIV_19		0x3
#define STM_LCD_FCR_DIV_20		0x4
#define STM_LCD_FCR_DIV_21		0x5
#define STM_LCD_FCR_DIV_22		0x6
#define STM_LCD_FCR_DIV_23		0x7
#define STM_LCD_FCR_DIV_24		0x8
#define STM_LCD_FCR_DIV_25		0x9
#define STM_LCD_FCR_DIV_26		0xa
#define STM_LCD_FCR_DIV_27		0xb
#define STM_LCD_FCR_DIV_28		0xc
#define STM_LCD_FCR_DIV_29		0xd
#define STM_LCD_FCR_DIV_30		0xe
#define STM_LCD_FCR_DIV_31		0xf
#define STM_LCD_FCR_DIV_MASK		0xf

#define STM_LCD_FCR_BLINK		(16)
#define  STM_LCD_FCR_BLINK_DISABLE		0
#define  STM_LCD_FCR_BLINK_SEG0_COM0		1
#define  STM_LCD_FCR_BLINK_SEG0_COMALL		2
#define  STM_LCD_FCR_BLINK_SEGALL_COMALL	3
#define  STM_LCD_FCR_BLINK_MASK			3

#define STM_LCD_FCR_BLINKF		(13)
#define  STM_LCD_FCR_BLINKF_8			0
#define  STM_LCD_FCR_BLINKF_16			1
#define  STM_LCD_FCR_BLINKF_32			2
#define  STM_LCD_FCR_BLINKF_64			3
#define  STM_LCD_FCR_BLINKF_128			4
#define  STM_LCD_FCR_BLINKF_256			5
#define  STM_LCD_FCR_BLINKF_512			6
#define  STM_LCD_FCR_BLINKF_1024		7
#define  STM_LCD_FCR_BLINKF_MASK		7

#define STM_LCD_FCR_CC			(10)
#define  STM_LCD_FCR_CC_MASK			7

#define STM_LCD_FCR_DEAD		(7)
#define  STM_LCD_FCR_DEAD_MASK			7

#define STM_LCD_FCR_PON			(4)
#define  STM_LCD_FCR_PON_MASK			7

#define STM_LCD_FCR_UDDIE		(3)
#define STM_LCD_FCR_SOFIE		(1)
#define STM_LCD_FCR_HD			(0)

#define STM_LCD_SR_FCRSF		(5)
#define STM_LCD_SR_RDY			(4)
#define STM_LCD_SR_UDD			(3)
#define STM_LCD_SR_UDR			(2)
#define STM_LCD_SR_SOF			(1)
#define STM_LCD_SR_ENS			(0)

#define STM_LCD_CLR_UDDC		(3)
#define STM_LCD_CLR_SOFC		(1)

struct stm_nvic {
	vuint32_t	iser[3];	/* 0x000 */

	uint8_t		_unused00c[0x080 - 0x00c];

	vuint32_t	icer[3];	/* 0x080 */

	uint8_t		_unused08c[0x100 - 0x08c];

	vuint32_t	ispr[3];	/* 0x100 */

	uint8_t		_unused10c[0x180 - 0x10c];

	vuint32_t	icpr[3];	/* 0x180 */

	uint8_t		_unused18c[0x200 - 0x18c];

	vuint32_t	iabr[3];	/* 0x200 */

	uint8_t		_unused20c[0x300 - 0x20c];

	vuint32_t	ipr[21];	/* 0x300 */

	uint8_t		_unused324[0xe00 - 0x324];

	vuint32_t	stir;		/* 0xe00 */
};

extern struct stm_nvic stm_nvic;

#define IRQ_REG(irq)	((irq) >> 5)
#define IRQ_BIT(irq)	((irq) & 0x1f)
#define IRQ_MASK(irq)	(1 << IRQ_BIT(irq))
#define IRQ_BOOL(v,irq)	(((v) >> IRQ_BIT(irq)) & 1)

static inline void
stm_nvic_set_enable(int irq) {
	stm_nvic.iser[IRQ_REG(irq)] = IRQ_MASK(irq);
}

static inline void
stm_nvic_clear_enable(int irq) {
	stm_nvic.icer[IRQ_REG(irq)] = IRQ_MASK(irq);
}

static inline int
stm_nvic_enabled(int irq) {
	return IRQ_BOOL(stm_nvic.iser[IRQ_REG(irq)], irq);
}
	
static inline void
stm_nvic_set_pending(int irq) {
	stm_nvic.ispr[IRQ_REG(irq)] = IRQ_MASK(irq);
}

static inline void
stm_nvic_clear_pending(int irq) {
	stm_nvic.icpr[IRQ_REG(irq)] = IRQ_MASK(irq);
}

static inline int
stm_nvic_pending(int irq) {
	return IRQ_BOOL(stm_nvic.ispr[IRQ_REG(irq)], irq);
}

static inline int
stm_nvic_active(int irq) {
	return IRQ_BOOL(stm_nvic.iabr[IRQ_REG(irq)], irq);
}

#define IRQ_PRIO_REG(irq)	((irq) >> 2)
#define IRQ_PRIO_BIT(irq)	(((irq) & 3) << 3)
#define IRQ_PRIO_MASK(irq)	(0xff << IRQ_PRIO_BIT(irq))

static inline void
stm_nvic_set_priority(int irq, uint8_t prio) {
	int		n = IRQ_PRIO_REG(irq);
	uint32_t	v;

	v = stm_nvic.ipr[n];
	v &= ~IRQ_PRIO_MASK(irq);
	v |= (prio) << IRQ_PRIO_BIT(irq);
	stm_nvic.ipr[n] = v;
}

static inline uint8_t
stm_nvic_get_priority(int irq) {
	return (stm_nvic.ipr[IRQ_PRIO_REG(irq)] >> IRQ_PRIO_BIT(irq)) & IRQ_PRIO_MASK(0);
}

#define isr(name) void stm_ ## name ## _isr(void);

isr(nmi)
isr(hardfault)
isr(memmanage)
isr(busfault)
isr(usagefault)
isr(svc)
isr(debugmon)
isr(pendsv)
isr(systick)
isr(wwdg)
isr(pvd)
isr(tamper_stamp)
isr(rtc_wkup)
isr(flash)
isr(rcc)
isr(exti0)
isr(exti1)
isr(exti2)
isr(exti3)
isr(exti4)
isr(dma1_channel1)
isr(dma1_channel2)
isr(dma1_channel3)
isr(dma1_channel4)
isr(dma1_channel5)
isr(dma1_channel6)
isr(dma1_channel7)
isr(adc1)
isr(usb_hp)
isr(usb_lp)
isr(dac)
isr(comp)
isr(exti9_5)
isr(lcd)
isr(tim9)
isr(tim10)
isr(tim11)
isr(tim2)
isr(tim3)
isr(tim4)
isr(i2c1_ev)
isr(i2c1_er)
isr(i2c2_ev)
isr(i2c2_er)
isr(spi1)
isr(spi2)
isr(usart1)
isr(usart2)
isr(usart3)
isr(exti15_10)
isr(rtc_alarm)
isr(usb_fs_wkup)
isr(tim6)
isr(tim7)

#undef isr

#define STM_ISR_WWDG_POS		0
#define STM_ISR_PVD_POS			1
#define STM_ISR_TAMPER_STAMP_POS	2
#define STM_ISR_RTC_WKUP_POS		3
#define STM_ISR_FLASH_POS		4
#define STM_ISR_RCC_POS			5
#define STM_ISR_EXTI0_POS		6
#define STM_ISR_EXTI1_POS		7
#define STM_ISR_EXTI2_POS		8
#define STM_ISR_EXTI3_POS		9
#define STM_ISR_EXTI4_POS		10
#define STM_ISR_DMA1_CHANNEL1_POS	11
#define STM_ISR_DMA2_CHANNEL1_POS	12
#define STM_ISR_DMA3_CHANNEL1_POS	13
#define STM_ISR_DMA4_CHANNEL1_POS	14
#define STM_ISR_DMA5_CHANNEL1_POS	15
#define STM_ISR_DMA6_CHANNEL1_POS	16
#define STM_ISR_DMA7_CHANNEL1_POS	17
#define STM_ISR_ADC1_POS		18
#define STM_ISR_USB_HP_POS		19
#define STM_ISR_USB_LP_POS		20
#define STM_ISR_DAC_POS			21
#define STM_ISR_COMP_POS		22
#define STM_ISR_EXTI9_5_POS		23
#define STM_ISR_LCD_POS			24
#define STM_ISR_TIM9_POS		25
#define STM_ISR_TIM10_POS		26
#define STM_ISR_TIM11_POS		27
#define STM_ISR_TIM2_POS		28
#define STM_ISR_TIM3_POS		29
#define STM_ISR_TIM4_POS		30
#define STM_ISR_I2C1_EV_POS		31
#define STM_ISR_I2C1_ER_POS		32
#define STM_ISR_I2C2_EV_POS		33
#define STM_ISR_I2C2_ER_POS		34
#define STM_ISR_SPI1_POS		35
#define STM_ISR_SPI2_POS		36
#define STM_ISR_USART1_POS		37
#define STM_ISR_USART2_POS		38
#define STM_ISR_USART3_POS		39
#define STM_ISR_EXTI15_10_POS		40
#define STM_ISR_RTC_ALARM_POS		41
#define STM_ISR_USB_FS_WKUP_POS		42
#define STM_ISR_TIM6_POS		43
#define STM_ISR_TIM7_POS		44

struct stm_dma_channel {
	vuint32_t	ccr;
	vuint32_t	cndtr;
	vvoid_t		cpar;
	vvoid_t		cmar;
	vuint32_t	reserved;
};

#define STM_NUM_DMA	7

struct stm_dma {
	vuint32_t		isr;
	vuint32_t		ifcr;
	struct stm_dma_channel	channel[STM_NUM_DMA];
};

extern struct stm_dma stm_dma;

/* DMA channels go from 1 to 7, instead of 0 to 6 (sigh)
 */

#define STM_DMA_INDEX(channel)		((channel) - 1)

#define STM_DMA_ISR(index)		((index) << 2)
#define STM_DMA_ISR_MASK			0xf
#define STM_DMA_ISR_TEIF			3
#define STM_DMA_ISR_HTIF			2
#define STM_DMA_ISR_TCIF			1
#define STM_DMA_ISR_GIF				0

#define STM_DMA_IFCR(index)		((index) << 2)
#define STM_DMA_IFCR_MASK			0xf
#define STM_DMA_IFCR_CTEIF      		3
#define STM_DMA_IFCR_CHTIF			2
#define STM_DMA_IFCR_CTCIF			1
#define STM_DMA_IFCR_CGIF			0

#define STM_DMA_CCR_MEM2MEM		(14)

#define STM_DMA_CCR_PL			(12)
#define  STM_DMA_CCR_PL_LOW			(0)
#define  STM_DMA_CCR_PL_MEDIUM			(1)
#define  STM_DMA_CCR_PL_HIGH			(2)
#define  STM_DMA_CCR_PL_VERY_HIGH		(3)
#define  STM_DMA_CCR_PL_MASK			(3)

#define STM_DMA_CCR_MSIZE		(10)
#define  STM_DMA_CCR_MSIZE_8			(0)
#define  STM_DMA_CCR_MSIZE_16			(1)
#define  STM_DMA_CCR_MSIZE_32			(2)
#define  STM_DMA_CCR_MSIZE_MASK			(3)

#define STM_DMA_CCR_PSIZE		(8)
#define  STM_DMA_CCR_PSIZE_8			(0)
#define  STM_DMA_CCR_PSIZE_16			(1)
#define  STM_DMA_CCR_PSIZE_32			(2)
#define  STM_DMA_CCR_PSIZE_MASK			(3)

#define STM_DMA_CCR_MINC		(7)
#define STM_DMA_CCR_PINC		(6)
#define STM_DMA_CCR_CIRC		(5)
#define STM_DMA_CCR_DIR			(4)
#define  STM_DMA_CCR_DIR_PER_TO_MEM		0
#define  STM_DMA_CCR_DIR_MEM_TO_PER		1
#define STM_DMA_CCR_TEIE		(3)
#define STM_DMA_CCR_HTIE		(2)
#define STM_DMA_CCR_TCIE		(1)
#define STM_DMA_CCR_EN			(0)

#define STM_DMA_CHANNEL_ADC1		1
#define STM_DMA_CHANNEL_SPI1_RX		2
#define STM_DMA_CHANNEL_SPI1_TX		3
#define STM_DMA_CHANNEL_SPI2_RX		4
#define STM_DMA_CHANNEL_SPI2_TX		5
#define STM_DMA_CHANNEL_USART3_TX	2
#define STM_DMA_CHANNEL_USART3_RX	3
#define STM_DMA_CHANNEL_USART1_TX	4
#define STM_DMA_CHANNEL_USART1_RX	5
#define STM_DMA_CHANNEL_USART2_RX	6
#define STM_DMA_CHANNEL_USART2_TX	7
#define STM_DMA_CHANNEL_I2C2_TX		4
#define STM_DMA_CHANNEL_I2C2_RX		5
#define STM_DMA_CHANNEL_I2C1_RX		6
#define STM_DMA_CHANNEL_I2C1_TX		7
#define STM_DMA_CHANNEL_TIM2_CH3	1
#define STM_DMA_CHANNEL_TIM2_UP		2
#define STM_DMA_CHANNEL_TIM2_CH1	5
#define STM_DMA_CHANNEL_TIM2_CH2	7
#define STM_DMA_CHANNEL_TIM2_CH4	7
#define STM_DMA_CHANNEL_TIM3_CH3	2
#define STM_DMA_CHANNEL_TIM3_CH4	3
#define STM_DMA_CHANNEL_TIM3_UP		3
#define STM_DMA_CHANNEL_TIM3_CH1	6
#define STM_DMA_CHANNEL_TIM3_TRIG	6
#define STM_DMA_CHANNEL_TIM4_CH1	1
#define STM_DMA_CHANNEL_TIM4_CH2	4
#define STM_DMA_CHANNEL_TIM4_CH3	5
#define STM_DMA_CHANNEL_TIM4_UP		7
#define STM_DMA_CHANNEL_TIM6_UP_DA	2
#define STM_DMA_CHANNEL_C_CHANNEL1	2
#define STM_DMA_CHANNEL_TIM7_UP_DA	3
#define STM_DMA_CHANNEL_C_CHANNEL2	3

/*
 * Only spi channel 1 and 2 can use DMA
 */
#define STM_NUM_SPI	2

struct stm_spi {
	vuint32_t	cr1;
	vuint32_t	cr2;
	vuint32_t	sr;
	vuint32_t	dr;
	vuint32_t	crcpr;
	vuint32_t	rxcrcr;
	vuint32_t	txcrcr;
};

extern struct stm_spi stm_spi1, stm_spi2, stm_spi3;

/* SPI channels go from 1 to 3, instead of 0 to 2 (sigh)
 */

#define STM_SPI_INDEX(channel)		((channel) - 1)

#define STM_SPI_CR1_BIDIMODE		15
#define STM_SPI_CR1_BIDIOE		14
#define STM_SPI_CR1_CRCEN		13
#define STM_SPI_CR1_CRCNEXT		12
#define STM_SPI_CR1_DFF			11
#define STM_SPI_CR1_RXONLY		10
#define STM_SPI_CR1_SSM			9
#define STM_SPI_CR1_SSI			8
#define STM_SPI_CR1_LSBFIRST		7
#define STM_SPI_CR1_SPE			6
#define STM_SPI_CR1_BR			3
#define  STM_SPI_CR1_BR_PCLK_2			0
#define  STM_SPI_CR1_BR_PCLK_4			1
#define  STM_SPI_CR1_BR_PCLK_8			2
#define  STM_SPI_CR1_BR_PCLK_16			3
#define  STM_SPI_CR1_BR_PCLK_32			4
#define  STM_SPI_CR1_BR_PCLK_64			5
#define  STM_SPI_CR1_BR_PCLK_128		6
#define  STM_SPI_CR1_BR_PCLK_256		7
#define  STM_SPI_CR1_BR_MASK			7

#define STM_SPI_CR1_MSTR		2
#define STM_SPI_CR1_CPOL		1
#define STM_SPI_CR1_CPHA		0

#define STM_SPI_CR2_TXEIE	7
#define STM_SPI_CR2_RXNEIE	6
#define STM_SPI_CR2_ERRIE	5
#define STM_SPI_CR2_SSOE	2
#define STM_SPI_CR2_TXDMAEN	1
#define STM_SPI_CR2_RXDMAEN	0

#define STM_SPI_SR_BSY		7
#define STM_SPI_SR_OVR		6
#define STM_SPI_SR_MODF		5
#define STM_SPI_SR_CRCERR	4
#define STM_SPI_SR_TXE		1
#define STM_SPI_SR_RXNE		0

struct stm_adc {
	vuint32_t	sr;
	vuint32_t	cr1;
	vuint32_t	cr2;
	vuint32_t	smpr1;
	vuint32_t	smpr2;
	vuint32_t	smpr3;
	vuint32_t	jofr1;
	vuint32_t	jofr2;
	vuint32_t	jofr3;
	vuint32_t	jofr4;
	vuint32_t	htr;
	vuint32_t	ltr;
	vuint32_t	sqr1;
	vuint32_t	sqr2;
	vuint32_t	sqr3;
	vuint32_t	sqr4;
	vuint32_t	sqr5;
	vuint32_t	jsqr;
	vuint32_t	jdr1;
	vuint32_t	jdr2;
	vuint32_t	jdr3;
	vuint32_t	jdr4;
	vuint32_t	dr;
	uint8_t		reserved[0x300 - 0x5c];
	vuint32_t	csr;
	vuint32_t	ccr;
};

extern struct stm_adc stm_adc;

#define STM_ADC_SR_JCNR		9
#define STM_ADC_SR_RCNR		8
#define STM_ADC_SR_ADONS	6
#define STM_ADC_SR_OVR		5
#define STM_ADC_SR_STRT		4
#define STM_ADC_SR_JSTRT	3
#define STM_ADC_SR_JEOC		2
#define STM_ADC_SR_EOC		1
#define STM_ADC_SR_AWD		0

#define STM_ADC_CR1_OVRIE	26
#define STM_ADC_CR1_RES		24
#define  STM_ADC_CR1_RES_12		0
#define  STM_ADC_CR1_RES_10		1
#define  STM_ADC_CR1_RES_8		2
#define  STM_ADC_CR1_RES_6		3
#define  STM_ADC_CR1_RES_MASK		3
#define STM_ADC_CR1_AWDEN       23
#define STM_ADC_CR1_JAWDEN	22
#define STM_ADC_CR1_PDI		17
#define STM_ADC_CR1_PDD		16
#define STM_ADC_CR1_DISCNUM	13
#define  STM_ADC_CR1_DISCNUM_1		0
#define  STM_ADC_CR1_DISCNUM_2		1
#define  STM_ADC_CR1_DISCNUM_3		2
#define  STM_ADC_CR1_DISCNUM_4		3
#define  STM_ADC_CR1_DISCNUM_5		4
#define  STM_ADC_CR1_DISCNUM_6		5
#define  STM_ADC_CR1_DISCNUM_7		6
#define  STM_ADC_CR1_DISCNUM_8		7
#define  STM_ADC_CR1_DISCNUM_MASK	7
#define STM_ADC_CR1_JDISCEN	12
#define STM_ADC_CR1_DISCEN	11
#define STM_ADC_CR1_JAUTO	10
#define STM_ADC_CR1_AWDSGL	9
#define STM_ADC_CR1_SCAN	8
#define STM_ADC_CR1_JEOCIE	7
#define STM_ADC_CR1_AWDIE	6
#define STM_ADC_CR1_EOCIE	5
#define STM_ADC_CR1_AWDCH	0
#define  STM_ADC_CR1_AWDCH_MASK		0x1f

#define STM_ADC_CR2_SWSTART	30
#define STM_ADC_CR2_EXTEN	28
#define  STM_ADC_CR2_EXTEN_DISABLE	0
#define  STM_ADC_CR2_EXTEN_RISING	1
#define  STM_ADC_CR2_EXTEN_FALLING	2
#define  STM_ADC_CR2_EXTEN_BOTH		3
#define  STM_ADC_CR2_EXTEN_MASK		3
#define STM_ADC_CR2_EXTSEL	24
#define  STM_ADC_CR2_EXTSEL_TIM9_CC2	0
#define  STM_ADC_CR2_EXTSEL_TIM9_TRGO	1
#define  STM_ADC_CR2_EXTSEL_TIM2_CC3	2
#define  STM_ADC_CR2_EXTSEL_TIM2_CC2	3
#define  STM_ADC_CR2_EXTSEL_TIM3_TRGO	4
#define  STM_ADC_CR2_EXTSEL_TIM4_CC4	5
#define  STM_ADC_CR2_EXTSEL_TIM2_TRGO	6
#define  STM_ADC_CR2_EXTSEL_TIM3_CC1	7
#define  STM_ADC_CR2_EXTSEL_TIM3_CC3	8
#define  STM_ADC_CR2_EXTSEL_TIM4_TRGO	9
#define  STM_ADC_CR2_EXTSEL_TIM6_TRGO	10
#define  STM_ADC_CR2_EXTSEL_EXTI_11	15
#define  STM_ADC_CR2_EXTSEL_MASK	15
#define STM_ADC_CR2_JWSTART	22
#define STM_ADC_CR2_JEXTEN	20
#define  STM_ADC_CR2_JEXTEN_DISABLE	0
#define  STM_ADC_CR2_JEXTEN_RISING	1
#define  STM_ADC_CR2_JEXTEN_FALLING	2
#define  STM_ADC_CR2_JEXTEN_BOTH	3
#define  STM_ADC_CR2_JEXTEN_MASK	3
#define STM_ADC_CR2_JEXTSEL	16
#define  STM_ADC_CR2_JEXTSEL_TIM9_CC1	0
#define  STM_ADC_CR2_JEXTSEL_TIM9_TRGO	1
#define  STM_ADC_CR2_JEXTSEL_TIM2_TRGO	2
#define  STM_ADC_CR2_JEXTSEL_TIM2_CC1	3
#define  STM_ADC_CR2_JEXTSEL_TIM3_CC4	4
#define  STM_ADC_CR2_JEXTSEL_TIM4_TRGO	5
#define  STM_ADC_CR2_JEXTSEL_TIM4_CC1	6
#define  STM_ADC_CR2_JEXTSEL_TIM4_CC2	7
#define  STM_ADC_CR2_JEXTSEL_TIM4_CC3	8
#define  STM_ADC_CR2_JEXTSEL_TIM10_CC1	9
#define  STM_ADC_CR2_JEXTSEL_TIM7_TRGO	10
#define  STM_ADC_CR2_JEXTSEL_EXTI_15	15
#define  STM_ADC_CR2_JEXTSEL_MASK	15
#define STM_ADC_CR2_ALIGN	11
#define STM_ADC_CR2_EOCS	10
#define STM_ADC_CR2_DDS		9
#define STM_ADC_CR2_DMA		8
#define STM_ADC_CR2_DELS	4
#define  STM_ADC_CR2_DELS_NONE		0
#define  STM_ADC_CR2_DELS_UNTIL_READ	1
#define  STM_ADC_CR2_DELS_7		2
#define  STM_ADC_CR2_DELS_15		3
#define  STM_ADC_CR2_DELS_31		4
#define  STM_ADC_CR2_DELS_63		5
#define  STM_ADC_CR2_DELS_127		6
#define  STM_ADC_CR2_DELS_255		7
#define  STM_ADC_CR2_DELS_MASK		7
#define STM_ADC_CR2_CONT	1
#define STM_ADC_CR2_ADON	0

#define STM_ADC_CCR_TSVREFE	23
#define STM_ADC_CCR_ADCPRE	16
#define  STM_ADC_CCR_ADCPRE_HSI_1	0
#define  STM_ADC_CCR_ADCPRE_HSI_2	1
#define  STM_ADC_CCR_ADCPRE_HSI_4	2
#define  STM_ADC_CCR_ADCPRE_MASK	3

struct stm_temp_cal {
	uint16_t	vref;
	uint16_t	ts_cal_cold;
	uint16_t	reserved;
	uint16_t	ts_cal_hot;
};

extern struct stm_temp_cal	stm_temp_cal;

#define stm_temp_cal_cold	25
#define stm_temp_cal_hot	110

#define STM_NUM_I2C	2

#define STM_I2C_INDEX(channel)	((channel) - 1)

struct stm_i2c {
	vuint32_t	cr1;
	vuint32_t	cr2;
	vuint32_t	oar1;
	vuint32_t	oar2;
	vuint32_t	dr;
	vuint32_t	sr1;
	vuint32_t	sr2;
	vuint32_t	ccr;
	vuint32_t	trise;
};

extern struct stm_i2c stm_i2c1, stm_i2c2;

#define STM_I2C_CR1_SWRST	15
#define STM_I2C_CR1_ALERT	13
#define STM_I2C_CR1_PEC		12
#define STM_I2C_CR1_POS		11
#define STM_I2C_CR1_ACK		10
#define STM_I2C_CR1_STOP	9
#define STM_I2C_CR1_START	8
#define STM_I2C_CR1_NOSTRETCH	7
#define STM_I2C_CR1_ENGC	6
#define STM_I2C_CR1_ENPEC	5
#define STM_I2C_CR1_ENARP	4
#define STM_I2C_CR1_SMBTYPE	3
#define STM_I2C_CR1_SMBUS	1
#define STM_I2C_CR1_PE		0

#define STM_I2C_CR2_LAST	12
#define STM_I2C_CR2_DMAEN	11
#define STM_I2C_CR2_ITBUFEN	10
#define STM_I2C_CR2_ITEVTEN	9
#define STM_I2C_CR2_ITERREN	8
#define STM_I2C_CR2_FREQ	0
#define  STM_I2C_CR2_FREQ_2_MHZ		2
#define  STM_I2C_CR2_FREQ_4_MHZ		4
#define  STM_I2C_CR2_FREQ_8_MHZ		8
#define  STM_I2C_CR2_FREQ_16_MHZ	16
#define  STM_I2C_CR2_FREQ_32_MHZ	32
#define  STM_I2C_CR2_FREQ_MASK		0x3f;

#define STM_I2C_SR1_SMBALERT	15
#define STM_I2C_SR1_TIMEOUT	14
#define STM_I2C_SR1_PECERR	12
#define STM_I2C_SR1_OVR		11
#define STM_I2C_SR1_AF		10
#define STM_I2C_SR1_ARLO	9
#define STM_I2C_SR1_BERR	8
#define STM_I2C_SR1_TXE		7
#define STM_I2C_SR1_RXNE	6
#define STM_I2C_SR1_STOPF	4
#define STM_I2C_SR1_ADD10	3
#define STM_I2C_SR1_BTF		2
#define STM_I2C_SR1_ADDR	1
#define STM_I2C_SR1_SB		0

#define STM_I2C_SR2_PEC		8
#define  STM_I2C_SR2_PEC_MASK	0xff00
#define STM_I2C_SR2_DUALF	7
#define STM_I2C_SR2_SMBHOST	6
#define STM_I2C_SR2_SMBDEFAULT	5
#define STM_I2C_SR2_GENCALL	4
#define STM_I2C_SR2_TRA		2
#define STM_I2C_SR2_BUSY       	1
#define STM_I2C_SR2_MSL		0

#define STM_I2C_CCR_FS		15
#define STM_I2C_CCR_DUTY	14
#define STM_I2C_CCR_CCR		0
#define  STM_I2C_CCR_MASK	0x7ff

#endif /* _STM32L_H_ */
