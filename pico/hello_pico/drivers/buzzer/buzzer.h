#ifndef BUZZER_H
#define BUZZER_H

#include "pico/stdlib.h"

// 初始化
void buzzer_init(uint pin);

// 開啟蜂鳴器
void buzzer_on();

// 關閉蜂鳴器
void buzzer_off();

// 嗶一聲
void buzzer_beep(int duration_ms);

#endif
