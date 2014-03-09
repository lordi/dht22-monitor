/**
 * DHT22 STM32F4 monitor
 * Author: Hannes Gräuler <hgraeule@uos.de>
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32_ub_lcd_ili9341.h"
#include "stm32_ub_graphic2d.h"
#include "stm32_ub_font.h"
#include "font/ub_font_arial_7x10.c"

#include "main.h"
#include "dht22.h"
#include "timer.h"

/*
 * UPDATE_INTERVAL (in ms) specifies the update interval and what time is
 * represented by a pixel
 */
#define UPDATE_INTERVAL (90*1000)

/* Length of the stored history (i.e. if UPDATE_INTERVAL is 1 minute, this is
 * the number of minutes that is displayed)
 * It also defines the width of the graph, so you probably don't want to change
 * it. */
#define DATA_LEN 256

/* Color for the two line graphs */
#define RGB_HUMID RGB_COL_YELLOW
#define RGB_TEMP  RGB_COL_MAGENTA

/* red, green, blue are 8-bit color components (0-255) */
uint16_t rgb565_from_triplet(uint8_t red, uint8_t green, uint8_t blue)
{
  red   >>= 3;
  green >>= 2;
  blue  >>= 3;
  return (red << 11) | (green << 5) | blue;
}

#ifdef DEBUG
void debug_init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
 
  // Clock Enable
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
 
  // Config PG13 als Digital-Ausgang, Debug LED
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
}
#endif 

void monitor_paint(uint16_t *data_humid, uint16_t *data_temp, uint16_t data_len, uint16_t current_index) {
    const int border = 5;
    const int width = data_len;
    const int height = 240 - 2*border;
    const int min_temp = 5;
    const int max_temp = 35;
    const int min_humid = 30;
    const int max_humid = 90;

    char buf[20];
    UB_LCD_FillLayer(0);

    uint16_t humid = data_humid[current_index];
    uint16_t temp = data_temp[current_index];


    int rgb_darkgrey = rgb565_from_triplet(32,32,32);
    int rgb_grey = rgb565_from_triplet(64,64,64);

    /* horizontal lines */
    int num_lines = 6;
    for (int y = 0; y < num_lines; y++) {
        int yy = (int)((y/(float)num_lines)*height);
        UB_Graphic2D_DrawLineNormal(border+yy+10, border, border+yy+10, border+width, rgb_grey);

        int temp2 = ((y/(float)num_lines)*(max_temp-min_temp)+min_temp)*10;
        sprintf(buf, "%2d C", (int)(temp2)/10);
        UB_Font_DrawString(border+yy,border+width+5,buf,&Arial_7x10,RGB_TEMP,RGB_COL_BLACK);
        temp2 = ((y/(float)num_lines)*(max_humid-min_humid)+min_humid)*10;
        sprintf(buf, "%2d %%RH", (int)(temp2)/10);
        UB_Font_DrawString(border+yy+10,border+width+5,buf,&Arial_7x10,RGB_HUMID,RGB_COL_BLACK);
    }

    /* vertical lines */
    const long hour = 60L*60L*1000L;
    int hour_pixels = hour/(UPDATE_INTERVAL);
    for (int x = width; x > 0; x -= hour_pixels) {
        UB_Graphic2D_DrawLineNormal(border, border+x, border+height, border+x, rgb_darkgrey);
    }

    UB_Graphic2D_DrawLineNormal(border, border, border+height, border, rgb_darkgrey);
    UB_Graphic2D_DrawLineNormal(border, border, border, border+width,  rgb_darkgrey);
    UB_Graphic2D_DrawLineNormal(border+height, border, border+height, border+width,  rgb_darkgrey);
    UB_Graphic2D_DrawLineNormal(border, border+width, border+height, border+width,  rgb_darkgrey);


    int y1_ = 0;
    int y2_ = 0;
    for (int x = 0; x < data_len; x++) {

        float y1c = ((data_humid[(x+current_index)%data_len]/10.0)-min_humid)/(max_humid-min_humid);
        float y2c = ((data_temp[(x+current_index)%data_len]/10.0)-min_temp)/(max_temp-min_temp);
        int y1 = y1c*height+10;
        int y2 = y2c*height+10;
        
        //UB_Graphic2D_DrawLineNormal(border+yy+10, border, border+yy+10, border+width, 0x0a0a);
        if (y1 > 0 && y1_ > 0)
            UB_Graphic2D_DrawLineNormal(border+y1_, border+x, border+y1, border+x+1, RGB_HUMID);
        if (y2 > 0 && y2_ > 0)
            UB_Graphic2D_DrawLineNormal(border+y2_, border+x, border+y2, border+x+1, RGB_TEMP);
        y1_ = y1;
        y2_ = y2;

    }
    sprintf(buf, "%2d.%d %%RH", (int)(humid)/10, ((int)(humid))%10);
    UB_Font_DrawString(220,70,buf,&Arial_7x10,RGB_HUMID,RGB_COL_BLACK);
    sprintf(buf, "%2d.%d C", (int)(temp)/10, ((int)(temp))%10);
    UB_Font_DrawString(220,10,buf,&Arial_7x10,RGB_TEMP, RGB_COL_BLACK);
}

/* This is just present because of a undefined ref linker error */
void _sbrk(void) {
}

int main(void)
{
  SystemInit(); // Quarz Einstellungen aktivieren
#ifdef DEBUG
  init();
#endif
  UB_LCD_Init();

  LCD_DISPLAY_MODE = LANDSCAPE;
  UB_LCD_LayerInit_Fullscreen();

  // auf Vordergrund schalten
  UB_LCD_SetLayer_2();

  timer_init();

  const uint16_t data_len = DATA_LEN;
  uint16_t data_humid[data_len];
  memset(data_humid, 0, sizeof(data_humid));
  uint16_t data_temp[data_len];
  memset(data_temp, 0, sizeof(data_temp));

  /* we store a pointer to "now" in the data arrays so that we do not have to
   * move them in memory */
  uint16_t current_index = 0;

  while(1)
  {
    timer_start(UPDATE_INTERVAL*1000L);
#ifdef DEBUG
    GPIOG->ODR |= GPIO_Pin_13; // PG13 an
#endif
    dht22_read();
    data_humid[current_index] = dht22_get_humidity();
    data_temp[current_index] = dht22_get_temp();
    monitor_paint(data_humid, data_temp, data_len, current_index);
    current_index = (current_index + 1) % data_len;
#ifdef DEBUG
    GPIOG->ODR ^= GPIO_Pin_13; // PG13 toggeln
#endif
    timer_block();
  }
}
