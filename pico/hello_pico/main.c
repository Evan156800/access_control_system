#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "drivers/ws2812/ws2812.h"
#include "drivers/sg90/sg90.h"

#define WS2812_PIN 2
#define SG90_PIN 15

int main() {
    stdio_init_all();

    PIO pio = pio0;
    uint sm = 0;

    // 初始化 WS2812（交給 driver）
    ws2812_init(pio, sm, WS2812_PIN);
    sg90_init(SG90_PIN);

    printf("Pico JSON Test Start\n");

        printf("0 deg\n");
    sg90_set_angle(SG90_PIN, 0);
    sleep_ms(2000);

    printf("90 deg\n");
    sg90_set_angle(SG90_PIN, 90);
    sleep_ms(2000);

    printf("180 deg\n");
    sg90_set_angle(SG90_PIN, 180);
    sleep_ms(2000);

    while (true) {

        // 模擬 Camera閒置
        const char *json_idle = "{\"device\":\"pico\",\"event\":\"pir\",\"value\":0}";
        printf("Send: %s\n", json_idle);
        ws2812_handle_event(json_idle);
	sg90_close(SG90_PIN);
        sleep_ms(1000);

        // 模擬 Camera偵測中   	
	const char *json_detect = "{\"device\":\"pico\",\"event\":\"pir\",\"value\":1}";
        printf("Send: %s\n", json_detect);
        ws2812_handle_event(json_detect);

        sleep_ms(1000);
	
	//模擬 辨識成功
	const char *json_pass = "{\"device\":\"pico\",\"event\":\"pir\",\"value\":2}";		
    	printf("Send: %s\n", json_pass);
        ws2812_handle_event(json_pass);
	
	sg90_open(SG90_PIN);   // ⭐ 核心：開門
        sleep_ms(3000);        // 開門維持
        sg90_close(SG90_PIN);  // 自動關門

        sleep_ms(1000);
	

        //模擬 辨識失敗
        const char *json_fail = "{\"device\":\"pico\",\"event\":\"pir\",\"value\":3}";
        printf("Send: %s\n", json_fail);
        ws2812_handle_event(json_fail);
	sg90_close(SG90_PIN);

        sleep_ms(1000);
	
    }
}
