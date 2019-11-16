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

#define MAX_SERVICES 20


const int relay1 = 2;
const int relay2 = 4;
const int relay3 = 5;
const int relay4 = 14;
const int relay5 = 1;
const int relay6 = 13;
const int relay7 = 12;
const int relay8 = 3;
bool relay1_on = false;
bool relay2_on = false;
bool relay3_on = false;
bool relay4_on = false;
bool relay5_on = false;
bool relay6_on = false;
bool relay7_on = false;
bool relay8_on = false;

void led_write(bool on) {
    gpio_write(relay1, on ? 0 : 1);
}

void led_write2(bool on) {
    gpio_write(relay2, on ? 0 : 1);
}

void led_write3(bool on) {
    gpio_write(relay3, on ? 0 : 1);
}

void led_write4(bool on) {
    gpio_write(relay4, on ? 0 : 1);
}

void led_write5(bool on) {
    gpio_write(relay5, on ? 0 : 1);
}

void led_write6(bool on) {
    gpio_write(relay6, on ? 0 : 1);
}

void led_write7(bool on) {
    gpio_write(relay7, on ? 0 : 1);
}

void led_write8(bool on) {
    gpio_write(relay8, on ? 0 : 1);
}

void led_init() {
    gpio_enable(relay1, GPIO_OUTPUT);
        gpio_enable(relay2, GPIO_OUTPUT);
        gpio_enable(relay3, GPIO_OUTPUT);
        gpio_enable(relay4, GPIO_OUTPUT);
        gpio_enable(relay5, GPIO_OUTPUT);
        gpio_enable(relay6, GPIO_OUTPUT);
        gpio_enable(relay7, GPIO_OUTPUT);
        gpio_enable(relay8, GPIO_OUTPUT);
    led_write(relay1_on);
    led_write2(relay2_on);
    led_write3(relay3_on);
    led_write4(relay4_on);
    led_write5(relay5_on);
    led_write6(relay6_on);
    led_write7(relay7_on);
    led_write8(relay8_on);
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

homekit_value_t relay2_on_get() {
    return HOMEKIT_BOOL(relay2_on);
}

homekit_value_t relay3_on_get() {
    return HOMEKIT_BOOL(relay3_on);
}

homekit_value_t relay4_on_get() {
    return HOMEKIT_BOOL(relay4_on);
}

homekit_value_t relay5_on_get() {
    return HOMEKIT_BOOL(relay5_on);
}

homekit_value_t relay6_on_get() {
    return HOMEKIT_BOOL(relay6_on);
}

homekit_value_t relay7_on_get() {
    return HOMEKIT_BOOL(relay7_on);
}

homekit_value_t relay8_on_get() {
    return HOMEKIT_BOOL(relay8_on);
}

void relay1_on_set(homekit_value_t value) {
    if (value.format != homekit_format_bool) {
        printf("Invalid value format: %d\n", value.format);
        return;
    }

    relay1_on = value.bool_value;
    led_write(relay1_on);
}

void relay2_on_set(homekit_value_t value) {
    if (value.format != homekit_format_bool) {
        printf("Invalid value format: %d\n", value.format);
        return;
    }

    relay2_on = value.bool_value;
    led_write2(relay2_on);
}

void relay3_on_set(homekit_value_t value) {
    if (value.format != homekit_format_bool) {
        printf("Invalid value format: %d\n", value.format);
        return;
    }

    relay3_on = value.bool_value;
    led_write3(relay3_on);
}

void relay4_on_set(homekit_value_t value) {
    if (value.format != homekit_format_bool) {
        printf("Invalid value format: %d\n", value.format);
        return;
    }

    relay4_on = value.bool_value;
    led_write4(relay4_on);
}

void relay5_on_set(homekit_value_t value) {
    if (value.format != homekit_format_bool) {
        printf("Invalid value format: %d\n", value.format);
        return;
    }

    relay5_on = value.bool_value;
    led_write5(relay5_on);
}

void relay6_on_set(homekit_value_t value) {
    if (value.format != homekit_format_bool) {
        printf("Invalid value format: %d\n", value.format);
        return;
    }

    relay6_on = value.bool_value;
    led_write6(relay6_on);
}

void relay7_on_set(homekit_value_t value) {
    if (value.format != homekit_format_bool) {
        printf("Invalid value format: %d\n", value.format);
        return;
    }

    relay7_on = value.bool_value;
    led_write7(relay7_on);
}

void relay8_on_set(homekit_value_t value) {
    if (value.format != homekit_format_bool) {
        printf("Invalid value format: %d\n", value.format);
        return;
    }

    relay8_on = value.bool_value;
    led_write8(relay8_on);
}

void init_accessory() {
    uint8_t macaddr[6];
    sdk_wifi_get_macaddr(STATION_IF, macaddr);

    int name_len = snprintf(NULL, 0, "Relays-%02X%02X%02X",
                            macaddr[3], macaddr[4], macaddr[5]);
    char *name_value = malloc(name_len+1);
    snprintf(name_value, name_len+1, "Relays-%02X%02X%02X",
             macaddr[3], macaddr[4], macaddr[5]);

    homekit_service_t* services[MAX_SERVICES + 8];
    homekit_service_t** s = services;

    *(s++) = NEW_HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
        NEW_HOMEKIT_CHARACTERISTIC(NAME, name_value),
        NEW_HOMEKIT_CHARACTERISTIC(MANUFACTURER, "HaPK"),
        NEW_HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0"),
        NEW_HOMEKIT_CHARACTERISTIC(MODEL, "Relays"),
        NEW_HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.1"),
        NEW_HOMEKIT_CHARACTERISTIC(IDENTIFY, lamp_identify),
        NULL
    });

    for (int i=0; i < relay_count; i++) {
        int relay_name_len = snprintf(NULL, 0, "Relay %d", i + 1);
        char *relay_name_value = malloc(relay_name_len+1);
        snprintf(relay_name_value, relay_name_len+1, "Relay %d", i + 1);

        *(s++) = NEW_HOMEKIT_SERVICE(LIGHTBULB, .characteristics=(homekit_characteristic_t*[]) {
            NEW_HOMEKIT_CHARACTERISTIC(NAME, relay_name_value),
            NEW_HOMEKIT_CHARACTERISTIC(
                ON, true,
                .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(
                    relay_callback, .context=(void*)&relay_gpios[i]
                ),
            ),
            NULL
        });
    }

    *(s++) = NULL;

    accessories[0] = NEW_HOMEKIT_ACCESSORY(.category=homekit_accessory_category_other, .services=services);
    accessories[1] = NULL;
}

void create_accessory_name() {
    uint8_t macaddr[6];
    sdk_wifi_get_macaddr(STATION_IF, macaddr);
    
    int name_len = snprintf(NULL, 0, "Sonoff Switch-%02X%02X%02X",
                            macaddr[3], macaddr[4], macaddr[5]);
    char *name_value = malloc(name_len+1);
    snprintf(name_value, name_len+1, "Sonoff Switch-%02X%02X%02X",
             macaddr[3], macaddr[4], macaddr[5]);
    
    name.value = HOMEKIT_STRING(name_value);
}


homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-111"
};

void on_wifi_ready() {
    homekit_server_init(&config);
}

void user_init(void) {
    uart_set_baud(0, 115200);

    wifi_config_init("Relay_8CH", NULL, on_wifi_ready);
    led_init();
}
