#ifndef PIR_HW_H
#define PIR_HW_H

#include <stdint.h>

/* =========================
   RP2040 SIO Register Base
   ========================= */
#define SIO_BASE        0xD0000000

/* GPIO input register */
#define SIO_GPIO_IN     (*(volatile uint32_t *)(SIO_BASE + 0x004))

/* =========================
   IO Bank (設定 GPIO 功能)
   ========================= */
#define IO_BANK0_BASE   0x40014000
#define GPIO_CTRL_OFFSET 0x04

/* 每個 GPIO 間隔 8 bytes */
#define GPIO_CTRL(gpio) (*(volatile uint32_t *)(IO_BANK0_BASE + 0x04 + (gpio * 8)))

/* Function select */
#define GPIO_FUNC_SIO   5

#endif
