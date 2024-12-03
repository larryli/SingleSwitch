#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iot_button.h"
#include "led_indicator.h"

static const char *TAG = "app_main";

static void button_single_click_cb(void *arg, void *usr_data)
{
    ESP_LOGI(TAG, "BUTTON_SINGLE_CLICK");
}

static led_indicator_handle_t led_handle = NULL;

#define LEDC

#ifdef LEDC
/**
 * @brief Define blinking type and priority.
 *
 */
enum {
    BLINK_DOUBLE = 0,
    BLINK_TRIPLE,
    BLINK_BRIGHT_75_PERCENT,
    BLINK_BRIGHT_25_PERCENT,
    BLINK_BREATHE_SLOW,
    BLINK_BREATHE_FAST,
    BLINK_MAX,
};

/**
 * @brief Blinking twice times has a priority level of 0 (highest).
 *
 */
static const blink_step_t double_blink[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_STOP, 0, 0},
};

/**
 * @brief Blinking three times has a priority level of 1.
 *
 */
static const blink_step_t triple_blink[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_STOP, 0, 0},
};

/**
 * @brief Brightness set to 75% with a priority level of 2.
 *
 */
static const blink_step_t bright_75_percent[] = {
    {LED_BLINK_BRIGHTNESS, LED_STATE_75_PERCENT, 0},
    {LED_BLINK_STOP, 0, 0},
};

/**
 * @brief Brightness set to 25% with a priority level of 3.
 *
 */
static const blink_step_t bright_25_percent[] = {
    {LED_BLINK_BRIGHTNESS, LED_STATE_25_PERCENT, 0},
    {LED_BLINK_STOP, 0, 0},
};

/**
 * @brief Slow breathing with a priority level of 4.
 *
 */
static const blink_step_t breath_slow_blink[] = {
    {LED_BLINK_HOLD, LED_STATE_OFF, 0},
    {LED_BLINK_BREATHE, LED_STATE_ON, 1000},
    {LED_BLINK_BREATHE, LED_STATE_OFF, 1000},
    {LED_BLINK_LOOP, 0, 0},
};

/**
 * @brief Fast breathing with a priority level of 5(lowest).
 *
 */
static const blink_step_t breath_fast_blink[] = {
    {LED_BLINK_HOLD, LED_STATE_OFF, 0},
    {LED_BLINK_BREATHE, LED_STATE_ON, 500},
    {LED_BLINK_BREATHE, LED_STATE_OFF, 500},
    {LED_BLINK_LOOP, 0, 0},
};

blink_step_t const *led_mode[] = {
    [BLINK_DOUBLE] = double_blink,
    [BLINK_TRIPLE] = triple_blink,
    [BLINK_BRIGHT_75_PERCENT] = bright_75_percent,
    [BLINK_BRIGHT_25_PERCENT] = bright_25_percent,
    [BLINK_BREATHE_SLOW] = breath_slow_blink,
    [BLINK_BREATHE_FAST] = breath_fast_blink,
    [BLINK_MAX] = NULL,
};
#else
/**
 * @brief Define blinking type and priority.
 *
 */
enum {
    BLINK_DOUBLE = 0,
    BLINK_TRIPLE,
    BLINK_SLOW,
    BLINK_FAST,
    BLINK_MAX,
};

/**
 * @brief Blinking twice times has a priority level of 0 (highest).
 *
 */
static const blink_step_t double_blink[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_STOP, 0, 0},
};

/**
 * @brief Blinking three times has a priority level of 1.
 *
 */
static const blink_step_t triple_blink[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_STOP, 0, 0},
};

/**
 * @brief Slow blinking takes priority level 2.
 *
 */
static const blink_step_t slow_blink[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 1000},
    {LED_BLINK_HOLD, LED_STATE_OFF, 1000},
    {LED_BLINK_LOOP, 0, 0},
};

/**
 * @brief Fast blinking has priority level 3(lowest).
 *
 */
static const blink_step_t fast_blink[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 100},
    {LED_BLINK_HOLD, LED_STATE_OFF, 100},
    {LED_BLINK_LOOP, 0, 0},
};

blink_step_t const *led_mode[] = {
    [BLINK_DOUBLE] = double_blink,
    [BLINK_TRIPLE] = triple_blink,
    [BLINK_SLOW] = slow_blink,
    [BLINK_FAST] = fast_blink,
    [BLINK_MAX] = NULL,
};
#endif

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

#ifdef LEDC
    led_indicator_ledc_config_t ledc_config = {
        .is_active_level_high = 0,
        .timer_inited = false,
        .timer_num = LEDC_TIMER_0,
        .gpio_num = 2,
        .channel = LEDC_CHANNEL_0,
    };

    const led_indicator_config_t config = {
        .mode = LED_LEDC_MODE,
        .led_indicator_ledc_config = &ledc_config,
        .blink_lists = led_mode,
        .blink_list_num = BLINK_MAX,
    };
#else
    led_indicator_gpio_config_t gpio_config = {
        .gpio_num = 2,
        .is_active_level_high = 0,
    };

    const led_indicator_config_t config = {
        .mode = LED_GPIO_MODE,
        .led_indicator_gpio_config = &gpio_config,
        .blink_lists = led_mode,
        .blink_list_num = BLINK_MAX,
    };
#endif
    led_handle = led_indicator_create(&config);
    assert(led_handle != NULL);
#ifdef LEDC
    ledc_fade_func_install(0);
#endif

    while (1) {
        for (int i = 0; i < BLINK_MAX; i++) {
            led_indicator_start(led_handle, i);
            ESP_LOGI(TAG, "start blink: %d", i);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            led_indicator_stop(led_handle, i);
            ESP_LOGI(TAG, "stop blink: %d", i);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}
