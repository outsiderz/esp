#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "wifi.h"

#include <dht/dht.h>


#ifndef SENSOR_PIN
#error SENSOR_PIN is not specified
#endif


static void wifi_init() {
    struct sdk_station_config wifi_config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASSWORD,
    };
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&wifi_config);
    sdk_wifi_station_connect();
}

const int relay1 = 16;
const int relay2 = 5;
const int relay3 = 0;
const int relay4 = 2;
bool relay1_on = false;
bool relay2_on = false;
bool relay3_on = false;
bool relay4_on = false;

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

void led_init() {
    gpio_enable(relay1, GPIO_OUTPUT);
	gpio_enable(relay2, GPIO_OUTPUT);
	gpio_enable(relay3, GPIO_OUTPUT);
	gpio_enable(relay4, GPIO_OUTPUT);
    led_write(relay1_on);
    led_write2(relay2_on);
	led_write3(relay3_on);
    led_write4(relay4_on);
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

void temperature_sensor_identify(homekit_value_t _value) {
    printf("Temperature sensor identify\n");
}

homekit_characteristic_t temperature = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 0);
homekit_characteristic_t humidity    = HOMEKIT_CHARACTERISTIC_(CURRENT_RELATIVE_HUMIDITY, 0);


void temperature_sensor_task(void *_args) {
    gpio_set_pullup(SENSOR_PIN, false, false);
    float humidity_value, temperature_value;
    while (1) {
        bool success = dht_read_float_data(
            DHT_TYPE_DHT11, SENSOR_PIN,
            &humidity_value, &temperature_value
        );
        if (success) {
            temperature.value.float_value = temperature_value;
            humidity.value.float_value = humidity_value;
            homekit_characteristic_notify(&temperature, HOMEKIT_FLOAT(temperature_value));
            homekit_characteristic_notify(&humidity, HOMEKIT_FLOAT(humidity_value));
        } else {
            printf("Couldnt read data from sensor\n");
        }
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void temperature_sensor_init() {
    xTaskCreate(temperature_sensor_task, "Temperatore Sensor", 256, NULL, 2, NULL);
}


homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_thermostat, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Relay_4CH&Temperature"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "ReD"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0012345"),
            HOMEKIT_CHARACTERISTIC(MODEL, "Relay_4CH&Temperature"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "2.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, temperature_sensor_identify),
            NULL
        }),
        HOMEKIT_SERVICE(TEMPERATURE_SENSOR, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Temperature Sensor"),
            &temperature,
            NULL
        }),
        HOMEKIT_SERVICE(HUMIDITY_SENSOR, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Humidity Sensor"),
            &humidity,
            NULL
        }),
		
        HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Relay1"),
            HOMEKIT_CHARACTERISTIC(
                ON, false,
                .getter=relay1_on_get,
                .setter=relay1_on_set
            ),
            NULL
        }),
		HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Relay2"),
            HOMEKIT_CHARACTERISTIC(
                ON, false,
                .getter=relay2_on_get,
                .setter=relay2_on_set
            ),
            NULL
        }),
			HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Relay3"),
            HOMEKIT_CHARACTERISTIC(
                ON, false,
                .getter=relay3_on_get,
                .setter=relay3_on_set
            ),
            NULL
        }),
			HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Relay4"),
            HOMEKIT_CHARACTERISTIC(
                ON, false,
                .getter=relay4_on_get,
                .setter=relay4_on_set
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

void user_init(void) {
    uart_set_baud(0, 115200);
    wifi_init();
	led_init();
    temperature_sensor_init();
    homekit_server_init(&config);
}
