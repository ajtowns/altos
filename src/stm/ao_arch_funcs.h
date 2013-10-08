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

#ifndef _AO_ARCH_FUNCS_H_
#define _AO_ARCH_FUNCS_H_

/* ao_spi_stm.c
 */

/* PCLK is set to 16MHz (HCLK 32MHz, APB prescaler 2) */

#define AO_SPI_SPEED_8MHz	STM_SPI_CR1_BR_PCLK_2
#define AO_SPI_SPEED_4MHz	STM_SPI_CR1_BR_PCLK_4
#define AO_SPI_SPEED_2MHz	STM_SPI_CR1_BR_PCLK_8
#define AO_SPI_SPEED_1MHz	STM_SPI_CR1_BR_PCLK_16
#define AO_SPI_SPEED_500kHz	STM_SPI_CR1_BR_PCLK_32
#define AO_SPI_SPEED_250kHz	STM_SPI_CR1_BR_PCLK_64
#define AO_SPI_SPEED_125kHz	STM_SPI_CR1_BR_PCLK_128
#define AO_SPI_SPEED_62500Hz	STM_SPI_CR1_BR_PCLK_256

#define AO_SPI_SPEED_FAST	AO_SPI_SPEED_8MHz

/* Companion bus wants something no faster than 200kHz */

#define AO_SPI_SPEED_200kHz	AO_SPI_SPEED_125kHz

#define AO_SPI_CONFIG_1		0x00
#define AO_SPI_1_CONFIG_PA5_PA6_PA7	AO_SPI_CONFIG_1
#define AO_SPI_2_CONFIG_PB13_PB14_PB15	AO_SPI_CONFIG_1

#define AO_SPI_CONFIG_2		0x04
#define AO_SPI_1_CONFIG_PB3_PB4_PB5	AO_SPI_CONFIG_2
#define AO_SPI_2_CONFIG_PD1_PD3_PD4	AO_SPI_CONFIG_2

#define AO_SPI_CONFIG_3		0x08
#define AO_SPI_1_CONFIG_PE13_PE14_PE15	AO_SPI_CONFIG_3

#define AO_SPI_CONFIG_NONE	0x0c

#define AO_SPI_INDEX_MASK	0x01
#define AO_SPI_CONFIG_MASK	0x0c

#define AO_SPI_1_PA5_PA6_PA7	(STM_SPI_INDEX(1) | AO_SPI_1_CONFIG_PA5_PA6_PA7)
#define AO_SPI_1_PB3_PB4_PB5	(STM_SPI_INDEX(1) | AO_SPI_1_CONFIG_PB3_PB4_PB5)
#define AO_SPI_1_PE13_PE14_PE15	(STM_SPI_INDEX(1) | AO_SPI_1_CONFIG_PE13_PE14_PE15)

#define AO_SPI_2_PB13_PB14_PB15	(STM_SPI_INDEX(2) | AO_SPI_2_CONFIG_PB13_PB14_PB15)
#define AO_SPI_2_PD1_PD3_PD4	(STM_SPI_INDEX(2) | AO_SPI_2_CONFIG_PD1_PD3_PD4)

#define AO_SPI_INDEX(id)	((id) & AO_SPI_INDEX_MASK)
#define AO_SPI_CONFIG(id)	((id) & AO_SPI_CONFIG_MASK)

void
ao_spi_get(uint8_t spi_index, uint32_t speed);

void
ao_spi_put(uint8_t spi_index);

void
ao_spi_send(void *block, uint16_t len, uint8_t spi_index);

void
ao_spi_send_fixed(uint8_t value, uint16_t len, uint8_t spi_index);

void
ao_spi_recv(void *block, uint16_t len, uint8_t spi_index);

void
ao_spi_duplex(void *out, void *in, uint16_t len, uint8_t spi_index);

extern uint16_t	ao_spi_speed[STM_NUM_SPI];

void
ao_spi_init(void);

#define ao_spi_set_cs(reg,mask) ((reg)->bsrr = ((uint32_t) (mask)) << 16)
#define ao_spi_clr_cs(reg,mask) ((reg)->bsrr = (mask))

#define ao_spi_get_mask(reg,mask,bus, speed) do {		\
		ao_spi_get(bus, speed);				\
		ao_spi_set_cs(reg,mask);			\
	} while (0)

#define ao_spi_put_mask(reg,mask,bus) do {	\
		ao_spi_clr_cs(reg,mask);	\
		ao_spi_put(bus);		\
	} while (0)

#define ao_spi_get_bit(reg,bit,pin,bus,speed) ao_spi_get_mask(reg,(1<<bit),bus,speed)
#define ao_spi_put_bit(reg,bit,pin,bus) ao_spi_put_mask(reg,(1<<bit),bus)

#define ao_enable_port(port) do {					\
		if ((port) == &stm_gpioa)				\
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOAEN); \
		else if ((port) == &stm_gpiob)				\
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN); \
		else if ((port) == &stm_gpioc)				\
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOCEN); \
		else if ((port) == &stm_gpiod)				\
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIODEN); \
		else if ((port) == &stm_gpioe)				\
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOEEN); \
	} while (0)

#define ao_disable_port(port) do {					\
		if ((port) == &stm_gpioa)				\
			stm_rcc.ahbenr &= ~(1 << STM_RCC_AHBENR_GPIOAEN); \
		else if ((port) == &stm_gpiob)				\
			stm_rcc.ahbenr &= ~(1 << STM_RCC_AHBENR_GPIOBEN); \
		else if ((port) == &stm_gpioc)				\
			stm_rcc.ahbenr &= ~(1 << STM_RCC_AHBENR_GPIOCEN); \
		else if ((port) == &stm_gpiod)				\
			stm_rcc.ahbenr &= ~(1 << STM_RCC_AHBENR_GPIODEN); \
		else if ((port) == &stm_gpioe)				\
			stm_rcc.ahbenr &= ~(1 << STM_RCC_AHBENR_GPIOEEN); \
	} while (0)


#define ao_gpio_set(port, bit, pin, v) stm_gpio_set(port, bit, v)

#define ao_gpio_get(port, bit, pin) stm_gpio_get(port, bit)

#define ao_enable_output(port,bit,pin,v) do {			\
		ao_enable_port(port);				\
		ao_gpio_set(port, bit, pin, v);			\
		stm_moder_set(port, bit, STM_MODER_OUTPUT);\
	} while (0)

#define ao_gpio_set_mode(port,bit,mode) do {				\
		if (mode == AO_EXTI_MODE_PULL_UP)			\
			stm_pupdr_set(port, bit, STM_PUPDR_PULL_UP);	\
		else if (mode == AO_EXTI_MODE_PULL_DOWN)		\
			stm_pupdr_set(port, bit, STM_PUPDR_PULL_DOWN);	\
		else							\
			stm_pupdr_set(port, bit, STM_PUPDR_NONE);	\
	} while (0)
	
#define ao_enable_input(port,bit,mode) do {				\
		ao_enable_port(port);					\
		stm_moder_set(port, bit, STM_MODER_INPUT);		\
		ao_gpio_set_mode(port, bit, mode);			\
	} while (0)

#define ao_enable_cs(port,bit) do {				\
		stm_gpio_set((port), bit, 1);			\
		stm_moder_set((port), bit, STM_MODER_OUTPUT);	\
	} while (0)

#define ao_spi_init_cs(port, mask) do {				\
		ao_enable_port(port);				\
		if ((mask) & 0x0001) ao_enable_cs(port, 0);	\
		if ((mask) & 0x0002) ao_enable_cs(port, 1);	\
		if ((mask) & 0x0004) ao_enable_cs(port, 2);	\
		if ((mask) & 0x0008) ao_enable_cs(port, 3);	\
		if ((mask) & 0x0010) ao_enable_cs(port, 4);	\
		if ((mask) & 0x0020) ao_enable_cs(port, 5);	\
		if ((mask) & 0x0040) ao_enable_cs(port, 6);	\
		if ((mask) & 0x0080) ao_enable_cs(port, 7);	\
		if ((mask) & 0x0100) ao_enable_cs(port, 8);	\
		if ((mask) & 0x0200) ao_enable_cs(port, 9);	\
		if ((mask) & 0x0400) ao_enable_cs(port, 10);\
		if ((mask) & 0x0800) ao_enable_cs(port, 11);\
		if ((mask) & 0x1000) ao_enable_cs(port, 12);\
		if ((mask) & 0x2000) ao_enable_cs(port, 13);\
		if ((mask) & 0x4000) ao_enable_cs(port, 14);\
		if ((mask) & 0x8000) ao_enable_cs(port, 15);\
	} while (0)

/* ao_dma_stm.c
 */

extern uint8_t ao_dma_done[STM_NUM_DMA];

void
ao_dma_set_transfer(uint8_t 		index,
		    volatile void	*peripheral,
		    void		*memory,
		    uint16_t		count,
		    uint32_t		ccr);

void
ao_dma_set_isr(uint8_t index, void (*isr)(int index));

void
ao_dma_start(uint8_t index);

void
ao_dma_done_transfer(uint8_t index);

void
ao_dma_abort(uint8_t index);

void
ao_dma_alloc(uint8_t index);

void
ao_dma_init(void);

/* ao_i2c_stm.c */

void
ao_i2c_get(uint8_t i2c_index);

uint8_t
ao_i2c_start(uint8_t i2c_index, uint16_t address);

void
ao_i2c_put(uint8_t i2c_index);

uint8_t
ao_i2c_send(void *block, uint16_t len, uint8_t i2c_index, uint8_t stop);

uint8_t
ao_i2c_recv(void *block, uint16_t len, uint8_t i2c_index, uint8_t stop);

void
ao_i2c_init(void);

/* ao_serial_stm.c */
struct ao_stm_usart {
	struct ao_fifo		rx_fifo;
	struct ao_fifo		tx_fifo;
	struct stm_usart	*reg;
	uint8_t			tx_started;
};

#if HAS_SERIAL_1
extern struct ao_stm_usart	ao_stm_usart1;
#endif

#if HAS_SERIAL_2
extern struct ao_stm_usart	ao_stm_usart2;
#endif

#if HAS_SERIAL_3
extern struct ao_stm_usart	ao_stm_usart3;
#endif

#define ARM_PUSH32(stack, val)	(*(--(stack)) = (val))

static inline uint32_t
ao_arch_irqsave(void) {
	uint32_t	primask;
	asm("mrs %0,primask" : "=&r" (primask));
	ao_arch_block_interrupts();
	return primask;
}

static inline void
ao_arch_irqrestore(uint32_t primask) {
	asm("msr primask,%0" : : "r" (primask));
}

static inline void
ao_arch_memory_barrier() {
	asm volatile("" ::: "memory");
}

#if HAS_TASK
static inline void
ao_arch_init_stack(struct ao_task *task, void *start)
{
	uint32_t	*sp = (uint32_t *) (task->stack + AO_STACK_SIZE);
	uint32_t	a = (uint32_t) start;
	int		i;

	/* Return address (goes into LR) */
	ARM_PUSH32(sp, a);

	/* Clear register values r0-r12 */
	i = 13;
	while (i--)
		ARM_PUSH32(sp, 0);

	/* APSR */
	ARM_PUSH32(sp, 0);

	/* PRIMASK with interrupts enabled */
	ARM_PUSH32(sp, 0);

	task->sp = sp;
}

static inline void ao_arch_save_regs(void) {
	/* Save general registers */
	asm("push {r0-r12,lr}\n");

	/* Save APSR */
	asm("mrs r0,apsr");
	asm("push {r0}");

	/* Save PRIMASK */
	asm("mrs r0,primask");
	asm("push {r0}");
}

static inline void ao_arch_save_stack(void) {
	uint32_t	*sp;
	asm("mov %0,sp" : "=&r" (sp) );
	ao_cur_task->sp = (sp);
	if ((uint8_t *) sp < &ao_cur_task->stack[0])
		ao_panic (AO_PANIC_STACK);
}

static inline void ao_arch_restore_stack(void) {
	uint32_t	sp;
	sp = (uint32_t) ao_cur_task->sp;

	/* Switch stacks */
	asm("mov sp, %0" : : "r" (sp) );

	/* Restore PRIMASK */
	asm("pop {r0}");
	asm("msr primask,r0");

	/* Restore APSR */
	asm("pop {r0}");
	asm("msr apsr_nczvq,r0");

	/* Restore general registers */
	asm("pop {r0-r12,lr}\n");

	/* Return to calling function */
	asm("bx lr");
}

#ifndef HAS_SAMPLE_PROFILE
#define HAS_SAMPLE_PROFILE 0
#endif

#if !HAS_SAMPLE_PROFILE
#define HAS_ARCH_START_SCHEDULER	1

static inline void ao_arch_start_scheduler(void) {
	uint32_t	sp;
	uint32_t	control;

	asm("mrs %0,msp" : "=&r" (sp));
	asm("msr psp,%0" : : "r" (sp));
	asm("mrs %0,control" : "=&r" (control));
	control |= (1 << 1);
	asm("msr control,%0" : : "r" (control));
	asm("isb");
}
#endif

#define ao_arch_isr_stack()

#endif

#define ao_arch_wait_interrupt() do {				\
		asm("\twfi\n");					\
		ao_arch_release_interrupts();			\
		asm(".global ao_idle_loc\nao_idle_loc:");	\
		ao_arch_block_interrupts();			\
	} while (0)

#define ao_arch_critical(b) do {				\
		ao_arch_block_interrupts();			\
		do { b } while (0);				\
		ao_arch_release_interrupts();			\
	} while (0)

#endif /* _AO_ARCH_FUNCS_H_ */
