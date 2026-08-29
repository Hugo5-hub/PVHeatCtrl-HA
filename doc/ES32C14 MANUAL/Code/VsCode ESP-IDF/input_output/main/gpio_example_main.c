/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"

/**
 * Brief:
 * This test code shows how to configure gpio and how to use gpio interrupt.
 *
 * GPIO status:
 * GPIO18: output (ESP32C2/ESP32H2 uses GPIO8 as the second output pin)
 * GPIO19: output (ESP32C2/ESP32H2 uses GPIO9 as the second output pin)
 * GPIO4:  input, pulled up, interrupt from rising edge and falling edge
 * GPIO5:  input, pulled up, interrupt from rising edge.
 *
 * Note. These are the default GPIO pins to be used in the example. You can
 * change IO pins in menuconfig.
 *
 * Test:
 * Connect GPIO18(8) with GPIO4
 * Connect GPIO19(9) with GPIO5
 * Generate pulses on GPIO18(8)/19(9), that triggers interrupt on GPIO4/5
 *
 */

#define output1       27
#define output2       14
#define output3       12
#define output4       13
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<output1) | (1ULL<<output2) | (1ULL<<output3) | (1ULL<<output4))

#define input1        19
#define input2        18
#define input3        5
#define input4        17
#define GPIO_INPUT_PIN_SEL  ((1ULL<<input1) | (1ULL<<input2) | (1ULL<<input3) | (1ULL<<input4))

uint8_t Value = 0;

void Get_Input_Value(void)
{
    uint8_t Value1 = 0;
    uint8_t Value2 = 0;
    if(gpio_get_level(input1) == 0){Value1 |= 0x01;}
	if(gpio_get_level(input2) == 0){Value1 |= 0x02;}
	if(gpio_get_level(input3) == 0){Value1 |= 0x04;}
	if(gpio_get_level(input4) == 0){Value1 |= 0x08;}
    vTaskDelay(20 / portTICK_PERIOD_MS);
    if(gpio_get_level(input1) == 0){Value2 |= 0x01;}
	if(gpio_get_level(input2) == 0){Value2 |= 0x02;}
	if(gpio_get_level(input3) == 0){Value2 |= 0x04;}
	if(gpio_get_level(input4) == 0){Value2 |= 0x08;}
    if(Value1 == Value2)
    {
        Value = Value1;
    }
}

void app_main(void)
{
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    while(1) {
        Get_Input_Value();
        if(Value & 0x01)
        {
            gpio_set_level(output1, 1);
        }
        else
        {
            gpio_set_level(output1, 0);
        }

        if(Value & 0x02)
        {
            gpio_set_level(output2, 1);
        }
        else
        {
            gpio_set_level(output2, 0);
        }

        if(Value & 0x04)
        {
            gpio_set_level(output3, 1);
        }
        else
        {
            gpio_set_level(output3, 0);
        }

        if(Value & 0x08)
        {
            gpio_set_level(output4, 1);
        }
        else
        {
            gpio_set_level(output4, 0);
        }
    }
}
