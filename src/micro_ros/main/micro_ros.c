#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include <uros_network_interfaces.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/bool.h>
#include <std_msgs/msg/int32_multi_array.h>
#include <std_msgs/msg/int32.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include "micro_ros.h"
#include "sensors.h"
#include "espnow_rx.h"

#ifdef CONFIG_MICRO_ROS_ESP_XRCE_DDS_MIDDLEWARE
#include <rmw_microros/rmw_microros.h>
#endif

#define RCCHECK(fn)                                                                                \
    do {                                                                                           \
        rcl_ret_t temp_rc = (fn);                                                                  \
        if (temp_rc != RCL_RET_OK) {                                                               \
            printf("Failed status on line %d: %d\n", __LINE__, (int)temp_rc);                      \
            printf("RCL error: %s\n", rcl_get_error_string().str);                                 \
            rcl_reset_error();                                                                     \
            vTaskDelete(NULL);                                                                     \
        }                                                                                          \
    } while (0)
#define RCSOFTCHECK(fn)                                                                            \
    {                                                                                              \
        rcl_ret_t temp_rc = fn;                                                                    \
        if ((temp_rc != RCL_RET_OK)) {                                                             \
            printf("Failed status on line %d: %d. Continuing.\n", __LINE__, (int)temp_rc);         \
        }                                                                                          \
    }

#define LED_GPIO 2

rcl_subscription_t horn_subscriber;
rcl_publisher_t sensors_publisher;
rcl_publisher_t joystick_publisher;
rcl_publisher_t button_publisher;
rcl_publisher_t battery_publisher;

std_msgs__msg__Bool horn_msg;
std_msgs__msg__Int32MultiArray sensors_msg;
std_msgs__msg__Int32MultiArray joystick_msg;
std_msgs__msg__Bool button_msg;
std_msgs__msg__Int32 battery_msg;

static int32_t sensors_msg_data[SENSORS_CHANNEL_COUNT];
static int32_t joystick_msg_data[ESPNOW_ADC_COUNT];

static void blink_twice(void);
static void horn_subscription_callback(const void* msgin);
static void sensors_timer_callback(rcl_timer_t* timer, int64_t last_call_time);
static void controller_timer_callback(rcl_timer_t* timer, int64_t last_call_time);
static void micro_ros_task(void* arg);

void micro_ros_init() {
    // horn and led gpio init
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0);

    // pin micro-ros task in APP_CPU to make PRO_CPU to deal with wifi:
    xTaskCreate(micro_ros_task, "uros_task", CONFIG_MICRO_ROS_APP_STACK, NULL,
                CONFIG_MICRO_ROS_APP_TASK_PRIO, NULL);
}

static void micro_ros_task(void* arg) {
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support;

    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    RCCHECK(rcl_init_options_init(&init_options, allocator));

#ifdef CONFIG_MICRO_ROS_ESP_XRCE_DDS_MIDDLEWARE
    rmw_init_options_t* rmw_options = rcl_init_options_get_rmw_init_options(&init_options);

    // Static Agent IP and port can be used instead of autodisvery.
    RCCHECK(rmw_uros_options_set_udp_address(CONFIG_MICRO_ROS_AGENT_IP, CONFIG_MICRO_ROS_AGENT_PORT,
                                             rmw_options));
    // RCCHECK(rmw_uros_discover_agent(rmw_options));
#endif

    // create init_options
    RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));

    // create node
    rcl_node_t node;
    RCCHECK(rclc_node_init_default(&node, "esp", "", &support));

    // Create sensors_publisher
    RCCHECK(rclc_publisher_init_default(&sensors_publisher, &node,
                                        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray),
                                        "esp/sensors"));

    // sensors data static allocation
    sensors_msg.data.data = sensors_msg_data;
    sensors_msg.data.size = SENSORS_CHANNEL_COUNT;
    sensors_msg.data.capacity = SENSORS_CHANNEL_COUNT;

    sensors_msg.layout.dim.data = NULL;
    sensors_msg.layout.dim.size = 0;
    sensors_msg.layout.dim.capacity = 0;
    sensors_msg.layout.data_offset = 0;

    // Create joystick_publisher
    RCCHECK(rclc_publisher_init_default(&joystick_publisher, &node,
                                        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray),
                                        "esp/joystick"));

    // joystick data static allocation
    joystick_msg.data.data = joystick_msg_data;
    joystick_msg.data.size = ESPNOW_ADC_COUNT;
    joystick_msg.data.capacity = ESPNOW_ADC_COUNT;

    joystick_msg.layout.dim.data = NULL;
    joystick_msg.layout.dim.size = 0;
    joystick_msg.layout.dim.capacity = 0;
    joystick_msg.layout.data_offset = 0;

    // Create button_publisher
    RCCHECK(rclc_publisher_init_default(
        &button_publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool), "esp/button"));

    // Create battery_publisher
    RCCHECK(rclc_publisher_init_default(&battery_publisher, &node,
                                        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
                                        "esp/battery"));

    // create sensors_timer
    rcl_timer_t sensors_timer;

    RCCHECK(rclc_timer_init_default2(&sensors_timer, &support, RCL_MS_TO_NS(100),
                                     sensors_timer_callback, true));

    // create controller_timer
    rcl_timer_t controller_timer;

    RCCHECK(rclc_timer_init_default2(&controller_timer, &support, RCL_MS_TO_NS(100),
                                     controller_timer_callback, true));

    // create horn_subscriber
    RCCHECK(rclc_subscription_init_default(
        &horn_subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool), "esp/horn"));

    // Create executor
    rclc_executor_t executor;

    RCCHECK(rclc_executor_init(&executor, &support.context, 3, &allocator));

    RCCHECK(rclc_executor_add_timer(&executor, &sensors_timer));
    RCCHECK(rclc_executor_add_timer(&executor, &controller_timer));
    RCCHECK(rclc_executor_add_subscription(&executor, &horn_subscriber, &horn_msg,
                                           &horn_subscription_callback, ON_NEW_DATA));

    while (1) {
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // free resources
    RCCHECK(rclc_executor_fini(&executor));
    RCCHECK(rcl_node_fini(&node));

    vTaskDelete(NULL);
}

static void blink_twice(void) {
    for (int i = 0; i < 2; i++) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(200));

        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

static void horn_subscription_callback(const void* msgin) {
    const std_msgs__msg__Bool* msg = (const std_msgs__msg__Bool*)msgin;

    if (msg->data) {
        printf("Horn command received: true\n");
        blink_twice();
    }
}

static void sensors_timer_callback(rcl_timer_t* timer, int64_t last_call_time) {
    (void)last_call_time;

    if (timer == NULL) {
        return;
    }

    sensors_sample_t sample;

    if (sensors_get_latest(&sample)) {
        for (int i = 0; i < SENSORS_CHANNEL_COUNT; i++) {
            sensors_msg.data.data[i] = sample.raw[i];
        }
        RCSOFTCHECK(rcl_publish(&sensors_publisher, &sensors_msg, NULL));
    }
}

static void controller_timer_callback(rcl_timer_t* timer, int64_t last_call_time) {
    (void)last_call_time;

    if (timer == NULL) {
        return;
    }

    espnow_packet_t packet;

    if (espnow_get_latest(&packet)) {
        for (int i = 0; i < ESPNOW_ADC_COUNT; i++) {
            joystick_msg.data.data[i] = packet.adc[i];
        }

        button_msg.data = packet.button;
        battery_msg.data = packet.battery_mv;

        RCSOFTCHECK(rcl_publish(&joystick_publisher, &joystick_msg, NULL));
        RCSOFTCHECK(rcl_publish(&button_publisher, &button_msg, NULL));
        RCSOFTCHECK(rcl_publish(&battery_publisher, &battery_msg, NULL));
    }
}