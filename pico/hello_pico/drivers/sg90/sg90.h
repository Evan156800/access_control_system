#ifndef SG90_H
#define SG90_H

#include "pico/stdlib.h"

// 初始化 SG90
void sg90_init(uint pin);

// 設定角度（⭐缺這個）
void sg90_set_angle(uint pin, float angle);

// 開門（旋轉到指定角度）
void sg90_open(uint pin);

// 關門
void sg90_close(uint pin);

#endif
