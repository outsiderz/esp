#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "wifi_config.h"



//The GPIO pin that motion sensor is connected.
const int motion_sensor_gpio = 2;


bool led_value = false; //This is to keep track of the LED status.

//Additional for Motion
homekit_characteristic_t motion_detected  = HOMEKIT_CHARACTERISTIC_(MOTION_DETECTED, 0);
homekit_characteristic_t currentAmbientLightLevel = HOMEKIT_CHARACTERISTIC_(CURRENT_AMBIENT_LIGHT_LEVEL, 0,.min_value = (float[]) {0},);
homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "Sonoff Switch");


#ifndef SENSOR_PIN
#error SENSOR_PIN is not specified
#endif

void identify_task(void *_args) {
    vTaskDelete(NULL);
}

void identify(homekit_value_t _value) {
    printf("identify\n");
    xTaskCreate(identify_task, "identify", 128, NULL, 2, NULL);
}

void led_write(bool on) {
    gpio_write(led_gpio, on ? 0 : 1);
	led_value = on; //Keep track of the status
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

HOMEKIT_ACCESSORY(.id=2, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Motion Sensor"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Alex_Khmelenko"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "03145154"),
            HOMEKIT_CHARACTERISTIC(MODEL, "MotionSensor"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "test"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, identify),
            NULL
        }),
        HOMEKIT_SERVICE(MOTION_SENSOR, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Motion Sensor"),
            &motion_detected,
            NULL
        }),
        NULL
    }),
    NULL
};



void create_accessory_name() {
    uint8_t macaddr[6];
    sdk_wifi_get_macaddr(STATION_IF, macaddr);

    int name_len = snprintf(NULL, 0, "Homekit Sensor-%02X%02X%02X",
                            macaddr[3], macaddr[4], macaddr[5]);
    char *name_value = malloc(name_len+1);
    snprintf(name_value, name_len+1, "Homekit Sensor-%02X%02X%02X",
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

    wifi_config_init("motion", NULL, on_wifi_ready);
    led_init();
}

}
