/*
*
* Copyright 2015-2018 Espressif Systems (Shanghai) PTE LTD
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include <string.h>
#include <esp_log.h>
#include <audio_board.h>
#include <i2s_stream.h>
#include <audio_stream.h>
#include <led_pattern.h>
#include <va_board.h>
#include <va_button.h>
#include <va_led.h>
#include <m5stackcore2_init.h>
#include <media_hal.h>
#include <media_hal_playback.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "tone.h"
#include "core2forAWS.h"
#include "axp192.h"
#include "ft6336u.h"
#include "bm8563.h"
#include "mpu6886.h"
#include "lvgl/lvgl.h"
#include "disp_driver.h"
#include "va_dsp.h"

#define VA_TAG "AUDIO_BOARD"

#define VA_ASSERT(a, format, b, ...) \
    if ((a) == NULL) { \
        ESP_LOGE(PLAT_TAG, format, ##__VA_ARGS__); \
        return b;\
    }
/*I2S related */
#define I2S_OUT_VOL_DEFAULT     60

extern SemaphoreHandle_t spi_mutex;
extern SemaphoreHandle_t xGuiSemaphore;
xSemaphoreHandle mic_state = NULL; /* To protect I2S writes from I2S uninstall */

bool ab_but_mute = false;

int but_cb_reg_handlr(int ui_but_evt)
{
	if(ui_but_evt == VA_BUTTON_TAP_TO_TALK)
	{
		if(Button_WasPressed(button_left))
			return 1;
	}

	if(ui_but_evt == VA_BUTTON_MIC_MUTE)
	{
		if(Button_WasPressed(button_middle))
			return 1;
	}
	
	if(ui_but_evt == VA_BUTTON_CUSTOM_1)
	{
		if(Button_WasPressed(button_right))
			return 1;
	}

    return 0;
}

esp_err_t va_board_button_init()
{
	//Initialize M5Core2 Touch Button driver
	Core2ForAWS_Button_Init();

	//Map Buttons to Alexa
    va_touch_button_init(but_cb_reg_handlr);

    // Add touch button labels
    lv_obj_t * ptt_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(ptt_label, "Push to Talk");
    lv_obj_align(ptt_label,NULL,LV_ALIGN_IN_BOTTOM_LEFT, 4, -4);

    lv_obj_t * mute_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(mute_label, "Mute");
    lv_obj_align(mute_label,NULL,LV_ALIGN_IN_BOTTOM_MID, 0, -4);
    
    return ESP_OK;
}

static void brightness_slider_event_cb(lv_obj_t * slider, lv_event_t event) {
    if(event == LV_EVENT_VALUE_CHANGED) {
        Core2ForAWS_LCD_SetBrightness(lv_slider_get_value(slider));
    }
}

void va_led_set_pwm(const uint32_t *led_value)
{
    for (uint8_t i = 0; i < 10; i++) {
        Core2ForAWS_Sk6812_SetColor(i, led_value[i%5]);
        Core2ForAWS_Sk6812_Show();
    }
}

static esp_err_t va_board_led_init()
{
    //Initialize LED Pattern : Linear-5
    led_pattern_config_t *ab_led_conf = NULL;
    led_pattern_init(&ab_led_conf);

    //Initialize M5Core2 LED Driver
    Core2ForAWS_Sk6812_Init();
    Axp192_SetGPIO1Mode(1);

    //Initialize Alexa specific LED module layer
    va_led_init((led_pattern_config_t *)ab_led_conf);
    ESP_LOGI(VA_TAG, "M5Stack Core2 for AWS IoT EduKit LED driver Initialized.");
    return ESP_OK;
}

void display_function()
{
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);

    lv_obj_t * heading_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(heading_label, "******ALEXA APP*******");
    lv_obj_align(heading_label,NULL,LV_ALIGN_IN_TOP_MID, 0, 6);

    lv_obj_t * brightness_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(brightness_label, "Screen brightness");
    lv_obj_set_pos(brightness_label, 4, 60);

    Core2ForAWS_LED_Enable(1);

    lv_obj_t * brightness_slider = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_set_width(brightness_slider, 136);
    lv_obj_align(brightness_slider, brightness_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_obj_set_event_cb(brightness_slider, brightness_slider_event_cb);
    lv_slider_set_value(brightness_slider, 70, LV_ANIM_OFF);
    lv_slider_set_range(brightness_slider, 30, 100);

    xSemaphoreGive(xGuiSemaphore);
}

int va_board_init()
{
    int ret;
    i2s_config_t i2s_cfg = {};
    i2s_pin_config_t ab_i2s_pin = {0};

    mic_state = xSemaphoreCreateMutex();

    media_hal_playback_cfg_t cfg = {
        .channels = M5STACKCORE2_DAC_OUT_CH_CNT,
        .sample_rate = SAMPLE_RATE_DAC,
        .i2s_port_num = I2S_PORT_DAC,
        .bits_per_sample = M5STACKCORE2_BITS_PER_SAMPLE,
    };
    media_hal_init_playback(&cfg);

    ESP_LOGI(VA_TAG, "Installing M5Stack Core2 for AWS IoT EduKit board i2s driver for Speaker");
    audio_board_i2s_init_default(&i2s_cfg);
    ret = i2s_driver_install(I2S_PORT_DAC, &i2s_cfg, 0, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(VA_TAG, "Error installing i2s driver for stream");
        return ret;
    } 
    
    // Write I2S0 pin config
    audio_board_i2s_pin_config(I2S_PORT_DAC, &ab_i2s_pin);
    i2s_set_pin(I2S_PORT_DAC, &ab_i2s_pin);

    static media_hal_config_t media_hal_conf = MEDIA_HAL_DEFAULT();
    media_hal_t* mhal_handle = media_hal_init(&media_hal_conf);
    
    //Initialize all M5Stack specific device drivers
    spi_mutex = xSemaphoreCreateMutex();
    lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); //Assuming max size of lv_color_t = 16bit, DISP_BUF_SIZE calculated from max horizontal display size 480
    lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); //Assuming max size of lv_color_t = 16bit, DISP_BUF_SIZE calculated from max horizontal display size 480

    Core2ForAWS_Init((void *)buf1, (void *)buf2);
    FT6336U_Init();
    Core2ForAWS_LCD_Init(buf1, buf2);
    va_board_led_init();
    BM8563_Init();
    MPU6886_Init();

    display_function();

    //disable Alexa tones (Start of Response / End of Response) for this half duplex board
    tone_set_sor_state(0);
    tone_set_eor_state(0);

    return ret;
}
