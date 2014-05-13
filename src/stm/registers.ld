stm_fsmc   = 0xa0000000;
stm_aes    = 0x50060000;
stm_dma    = 0x40026000;
stm_flash  = 0x40023c00;
stm_rcc    = 0x40023800;
stm_crc    = 0x40023000;
stm_gpioh  = 0x40021400;
stm_gpioe  = 0x40021000;
stm_gpiod  = 0x40020c00;
stm_gpioc  = 0x40020800;
stm_gpiob  = 0x40020400;
stm_gpioa  = 0x40020000;
stm_usart1 = 0x40013800;
stm_spi1   = 0x40013000;
stm_sdio   = 0x40012c00;
stm_adc    = 0x40012400;
stm_tim11  = 0x40011000;
stm_tim10  = 0x40010c00;
stm_tim9   = 0x40010800;
stm_exti   = 0x40010400;
stm_syscfg = 0x40010000;
stm_comp   = 0x40007c00;
stm_ri     = 0x40007c04;
stm_dac    = 0x40007400;
stm_pwr    = 0x40007000;
stm_usb_sram = 0x40006000;
stm_usb    = 0x40005c00;
stm_i2c2   = 0x40005800;
stm_i2c1   = 0x40005400;
stm_usart5 = 0x40005000;
stm_usart4 = 0x40004c00;
stm_usart3 = 0x40004800;
stm_usart2 = 0x40004400;
stm_spi3   = 0x40003c00;	/* docs are broken here */
stm_spi2   = 0x40003800;	/* docs are broken here */
stm_iwdg   = 0x40003000;
stm_wwdg   = 0x40002c00;
stm_rtc    = 0x40002800;
stm_lcd    = 0x40002400;
stm_tim7   = 0x40001400;
stm_tim6   = 0x40001000;
stm_tim5   = 0x40000c00;
stm_tim4   = 0x40000800;
stm_tim3   = 0x40000400;
stm_tim2   = 0x40000000;

stm_systick = 0xe000e010;

stm_nvic   = 0xe000e100;

stm_scb    = 0xe000ed00;

stm_mpu    = 0xe000ed90;

stm_dbg_mcu = 0xe0042000;

/* calibration data in system memory */
stm_temp_cal = 0x1ff80078;
stm_flash_size_medium = 0x1ff8004c;
stm_flash_size_large = 0x1ff800cc;
stm_device_id = 0x1ff80050;
