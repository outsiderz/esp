#include <stdio.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_common.h>

#include <etstimer.h>
#include <esplibs/libmain.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <wifi_config.h>

#define MOTION_SENSOR_GPIO 14

#define DEBOUNCE_TIME 500 / portTICK_PERIOD_MS
#define RESET_TIME 10000

static ETSTimer device_restart_timer;

homekit_characteristic_t motion_detected = HOMEKIT_CHARACTERISTIC_(MOTION_DETECTED, 0);

void device_restart() {
sdk_system_restart();
}

void reset_call() {
homekit_server_reset();
wifi_config_reset();

sdk_os_timer_setfn (& device_restart_timer, device_restart, NULL);
sdk_os_timer_arm(&device_restart_timer, 5500, 0);

}

void motion_sensor_callback(uint8_t gpio) {


if (gpio == MOTION_SENSOR_GPIO){
    int new = 0;
    new = gpio_read(MOTION_SENSOR_GPIO);
    motion_detected.value = HOMEKIT_BOOL(new);
    homekit_characteristic_notify(&motion_detected, HOMEKIT_BOOL(new));
    printf("Motion Detected on %d\n", gpio);
}
else {
    printf("Interrupt on %d", gpio);
}

}

void gpio_init() {
gpio_enable(MOTION_SENSOR_GPIO, GPIO_INPUT);
gpio_set_pullup(MOTION_SENSOR_GPIO, true, true);
gpio_set_interrupt(MOTION_SENSOR_GPIO, GPIO_INTTYPE_EDGE_ANY, motion_sensor_callback);
}

void identify_task(void *_args) {
vTaskDelete(NULL);
}

void identify(homekit_value_t _value) {
printf("identify\n");
xTaskCreate(identify_task, "identify", 128, NULL, 2, NULL);
}

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "ESP8266");
homekit_characteristic_t serial = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, "344000");

homekit_accessory_t *accessories[] = {
HOMEKIT_ACCESSORY(
  .id=10,
  .category=homekit_accessory_category_sensor,
        .services=(homekit_service_t*[]) {
    HOMEKIT_SERVICE(
        ACCESSORY_INFORMATION,
        .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "motion_sensor"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Alex_Khmelenko"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "344000"),
            HOMEKIT_CHARACTERISTIC(MODEL, "motion"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.1"),
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

homekit_server_config_t config = {
.accessories = accessories,
.password = "111-11-111"
};

void create_accessory_name() {
uint8_t macaddr[6];
sdk_wifi_get_macaddr(STATION_IF, macaddr);


char *name_value = malloc(14);
snprintf(name_value, 14, "Homekit-%02X%02X%02X", macaddr[3], macaddr[4], macaddr[5]);

name.value = HOMEKIT_STRING(name_value);
serial.value = name.value;

}

void on_wifi_ready() {
create_accessory_name();
}

void user_init(void) {
uart_set_baud(0, 115200);


wifi_config_init("MOTION_SENSOR", NULL, on_wifi_ready);
homekit_server_init(&config);
gpio_init();

}
