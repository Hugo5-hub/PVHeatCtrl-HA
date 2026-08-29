/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/dac_oneshot.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_check.h"

void app_main(void)
{
    /* DAC oneshot init */
    dac_oneshot_handle_t chan0_handle;
    dac_oneshot_config_t chan0_cfg = {
        .chan_id = DAC_CHAN_0,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan0_cfg, &chan0_handle));

    dac_oneshot_handle_t chan1_handle;
    dac_oneshot_config_t chan1_cfg = {
        .chan_id = DAC_CHAN_1,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan1_cfg, &chan1_handle));

    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan0_handle, 127));
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan1_handle, 255));
}
