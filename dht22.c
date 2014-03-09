/*
 * DHT22 driver for the STM32F429
 */
#include "inttypes.h"
#include <string.h>

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "dht22.h"
#include "timer.h"

/* Config */
#define DHT22_GPIO	GPIOG
#define DHT22_PIN GPIO_Pin_5

/* Globals */
static uint8_t dht_bytes[5];

/* Implementation */

/* read values from DHT22, return SUCCESS or ERROR */
int dht22_read() {

    memset(&dht_bytes, 0, 5);

    /* send startup sequence, see DHT22 digital protocol */
    dht22_start();

    int odr = GPIO_ReadInputDataBit(DHT22_GPIO, DHT22_PIN);
    int state = odr ? 1 : 0;
    signed int cnt = -1;
    int measure_start = timer_get();
    int last_measure_start = measure_start;
    int cur_timer = measure_start;

    while (cnt <= 41 && cur_timer > 0 && (measure_start - cur_timer) < 300000L) {

        int odr = GPIO_ReadInputDataBit(DHT22_GPIO, DHT22_PIN);
        if (odr && state == 0) {
            last_measure_start = cur_timer;
        }
        if (!odr && state == 1) {
            if (cnt >= 0) {
                int num = (cnt) / 8;
                dht_bytes[num] <<= 1;
                if ((last_measure_start - cur_timer) > 50)
                    dht_bytes[num] |= 1;
            }
            cnt++;
        }
        state = odr ? 1 : 0;
        cur_timer = timer_get();
    }

    return dht22_check_checksum() ? SUCCESS : ERROR;
}

void dht22_start(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // Clock Enable
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    // configure pin
    GPIO_InitStructure.GPIO_Pin = DHT22_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(DHT22_GPIO, &GPIO_InitStructure);

    GPIO_ResetBits(DHT22_GPIO, DHT22_PIN);
    timer_delay(500L);

    GPIO_SetBits(DHT22_GPIO, DHT22_PIN);
    timer_delay(40L);

    // switch to input
    GPIO_InitStructure.GPIO_Pin = DHT22_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DHT22_GPIO, &GPIO_InitStructure);

}

uint16_t dht22_get_humidity() {
    return (dht_bytes[0] << 8 | dht_bytes[1]);
}

uint16_t dht22_get_temp() {
    return (dht_bytes[2] << 8 | dht_bytes[3]);
}

int dht22_check_checksum() {
    uint8_t chksum = (dht_bytes[0] + dht_bytes[1] + dht_bytes[2] + dht_bytes[3]);
    return chksum == dht_bytes[4];
}
