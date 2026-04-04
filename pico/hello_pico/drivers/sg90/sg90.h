#ifndef SG90_H
#define SG90_H

#include "pico/stdlib.h"

// 初始化 SG90
void sg90_init(uint pin);

// 開門（旋轉到指定角度）
void sg90_open();

// 關門
void sg90_close();

#endif
