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


static void wifi_init() {
    struct sdk_station_config wifi_config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASSWORD,
    };

    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&wifi_config);
    sdk_wifi_station_connect();
}

const int relay1 = 12;
const int motion_sensor_gpio = 14 ;

bool relay1_on = false;

homekit_characteristic_t motion_detected  = HOMEKIT_CHARACTERISTIC_(MOTION_DETECTED, 0);
homekit_characteristic_t currentAmbientLightLevel = HOMEKIT_CHARACTERISTIC_(CURRENT_AMBIENT_LIGHT_LEVEL, 0,.min_value = (float[]) {0},);
homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "Sonoff Switch");


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


void motion_sensor_callback(uint8_t gpio) {

    if (gpio == motion_sensor_gpio){

		  uint16_t val = sdk_system_adc_read();
   			//float val2 = (3.2 / 1023.0);
			//val2 = val * val2;
				//printf ("ADC voltage is %f\n",  val2);
				printf ("ADC voltage is %d\n",  val);
		//printf ("ADC voltage is %.3f\n", sdk_system_adc_read());
		//currentAmbientLightLevel = val;

		//homekit_characteristic_notify(&currentAmbientLightLevel, HOMEKIT_FLOAT(val));

        int new = 0;
        new = gpio_read(motion_sensor_gpio);
        motion_detected.value = HOMEKIT_BOOL(new);

	led_write(new);


        homekit_characteristic_notify(&motion_detected, HOMEKIT_BOOL(new));
        printf("Motion Detected on %d\n", gpio);
    }
    else {
        printf("Interrupt on %d", gpio);

    }

}



homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Relay_4CH"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "ReD"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "037A2BABF19D"),
            HOMEKIT_CHARACTERISTIC(MODEL, "Relay_4CH"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.1"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, led_identify),
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
    homekit_server_init(&config);
}
