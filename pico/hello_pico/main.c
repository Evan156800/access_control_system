#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "drivers/ws2812/ws2812.h"
#include "drivers/sg90/sg90.h"
#include "drivers/buzzer/buzzer.h"
#include "drivers/pir/pir.h"

#define WS2812_PIN 2
#define PIR_PIN 14
#define SG90_PIN 15
#define BUZZER_PIN 16

int main() {
    stdio_init_all();

    PIO pio = pio0;
    uint sm = 0;

    // 初始化
    pir_init(PIR_PIN);
    ws2812_init(pio, sm, WS2812_PIN);
    sg90_init(SG90_PIN);
    buzzer_init(BUZZER_PIN);
    printf("Pico JSON Test Start\n");

    while (true) {
	int pir_value = pir_read();
        // 模擬 Camera閒置
	if(pir_value == 0){
        const char *json_idle = "{\"device\":\"pico\",\"event\":\"pir\",\"value\":0}";
        printf("Send: %s\n", json_idle);
        ws2812_handle_event(json_idle);
	sg90_close(SG90_PIN);
	//sleep_ms(1000);
	}
	else{
        // 模擬 Camera偵測中   	
	const char *json_detect = "{\"device\":\"pico\",\"event\":\"pir\",\"value\":1}";
        printf("Send: %s\n", json_detect);
        ws2812_handle_event(json_detect);

        sleep_ms(1000);
	
	//模擬 辨識成功
	const char *json_pass = "{\"device\":\"pico\",\"event\":\"pir\",\"value\":2}";		
    	printf("Send: %s\n", json_pass);
        ws2812_handle_event(json_pass);
	
	buzzer_beep(200);	
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
}
