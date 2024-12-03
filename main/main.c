#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iot_button.h"

static const char *TAG = "app_main";

static void button_single_click_cb(void *arg, void *usr_data)
{
    ESP_LOGI(TAG, "BUTTON_SINGLE_CLICK");
}

void app_main(void)
{
    ESP_LOGI(TAG, "start");

    button_config_t gpio_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
        .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
        .gpio_button_config =
            {
                .gpio_num = 0,
                .active_level = 0,
            },
    };
    button_handle_t gpio_btn = iot_button_create(&gpio_btn_cfg);
    if (NULL == gpio_btn) {
        ESP_LOGE(TAG, "Button create failed");
    }
    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK,
                           button_single_click_cb, NULL);

}
