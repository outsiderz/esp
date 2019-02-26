#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <wifi_config.h>

const int relay1 = 2;

bool relay1_on = false;

void led_write(bool on) {
    gpio_write(relay1, on ? 0 : 1);
}


void led_init() {
    gpio_enable(relay1, GPIO_OUTPUT);

    led_write(relay1_on);

}


void led_identify_task(void *_args) {
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            led_write(true);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            led_write(false);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    led_write(relay1_on);

    vTaskDelete(NULL);
}


void led_identify(homekit_value_t _value) {
    printf("LED identify\n");
    xTaskCreate(led_identify_task, "LED identify", 128, NULL, 2, NULL);
}



homekit_value_t relay1_on_get() {
    return HOMEKIT_BOOL(relay1_on);
}



void relay1_on_set(homekit_value_t value) {
    if (value.format != homekit_format_bool) {
        printf("Invalid value format: %d\n", value.format);
        return;
    }

    relay1_on = value.bool_value;
    led_write(relay1_on);
}


homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=6, .category=homekit_accessory_category_door_lock, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Relay_4CH"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "ReD from Alex_Khmelenko"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "037A2BABF19D"),
            HOMEKIT_CHARACTERISTIC(MODEL, "Relay_4CH"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.2"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, led_identify),
            NULL
        }),
        HOMEKIT_SERVICE(HOMEKIT_SERVICE_LOCK_MANAGEMENT, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(LOCK_CONTROL_POINT, "Relay1"),
            HOMEKIT_CHARACTERISTIC(
                ON, false,
                .getter=relay1_on_get,
                .setter=relay1_on_set
            ),
            NULL
        }),

        NULL
    }),
    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-111"
};

void on_wifi_ready() {
    homekit_server_init(&config);
}

void user_init(void) {
    uart_set_baud(0, 115200);

    wifi_config_init("Relay_4CH", NULL, on_wifi_ready);
    led_init();
}
