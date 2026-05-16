#include <zephyr/kernel.h>  /*includes zephyr Kernel API's*/
#include <zephyr/device.h>  /*Required to include API's like DEVICE_DT_GET()*/
#include <zephyr/drivers/sensor.h>  /*includes Zephyr’s generic sensor driver API*/
#include <zephyr/logging/log.h>    
#include "sensor_data.h"
#include <stdlib.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);        
K_MSGQ_DEFINE(sensor_msgq, sizeof(struct sensor_data), 10, 4);
#define PRODUCER_STACK_SIZE 1024
#define THREAD_PRIORITY 5
enum system_state {
    NORMAL,
    WARNING,
    FAULT
};
static int sensor_value_to_milli_int(struct sensor_value *val)
{
    return (val->val1 * 1000) + (val->val2 / 1000);
}
void producer_thread(void)
{
    k_sleep(K_SECONDS(1));        /*VERY IMPORTANT TO GIVE THE SENSORS A SECOND TO POWER UP AND INNITIALIZE!!!*/

    const struct device *bme = DEVICE_DT_GET(DT_NODELABEL(bme280));
    const struct device *mpu = DEVICE_DT_GET(DT_NODELABEL(mpu6050));

    if (!device_is_ready(bme)) {
        LOG_ERR("BME280 device not ready");
        return;
    }

    if (!device_is_ready(mpu)) {
        LOG_ERR("MPU6050 device not ready");
        return;
    }

    LOG_INF("Producer: sensors ready");

    while (1) {
        struct sensor_value temp;
        struct sensor_value accel[3];
        struct sensor_data data;

        int ret = sensor_sample_fetch(bme);
        if (ret < 0) {
            LOG_ERR("Producer: failed to fetch BME280, ret=%d", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }

        ret = sensor_channel_get(bme, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        if (ret < 0) {
            LOG_ERR("Producer: failed to read temperature, ret=%d", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }

        ret = sensor_sample_fetch(mpu);
        if (ret < 0) {
            LOG_ERR("Producer: failed to fetch MPU6050, ret=%d", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }

        ret = sensor_channel_get(mpu, SENSOR_CHAN_ACCEL_XYZ, accel);
        if (ret < 0) {
            LOG_ERR("Producer: failed to read accel, ret=%d", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }

        data.temperature_c = temp.val1;
        data.accel_x_milli = sensor_value_to_milli_int(&accel[0]);
        data.accel_y_milli = sensor_value_to_milli_int(&accel[1]);
        data.accel_z_milli = sensor_value_to_milli_int(&accel[2]);

        data.vibration_milli =
    abs(data.accel_x_milli) +
    abs(data.accel_y_milli) +
    abs(data.accel_z_milli);
        if (k_msgq_put(&sensor_msgq, &data, K_NO_WAIT) != 0) {
    LOG_WRN("Producer: queue full, data dropped");
}
        k_sleep(K_SECONDS(1));
    }
}
enum system_state check_system_state(struct sensor_data data)
{
    if (data.temperature_c > 35 || data.vibration_milli > 22000) {
        return FAULT;
    }

    if (data.temperature_c > 30 || data.vibration_milli > 18000) {
        return WARNING;
    }

    return NORMAL;
}
void consumer_thread(void)
{
    struct sensor_data data;
static enum system_state previous_state = NORMAL;

    while (1) {
        if (k_msgq_get(&sensor_msgq, &data, K_FOREVER) == 0) {
            enum system_state state = check_system_state(data);

LOG_INF("Consumer: temp=%d C vib=%d accel=(%d,%d,%d)",
        data.temperature_c,
        data.vibration_milli,
        data.accel_x_milli,
        data.accel_y_milli,
        data.accel_z_milli);

if (state != previous_state) {
    if (state == FAULT) {
        LOG_ERR("State changed: FAULT");
    } else if (state == WARNING) {
        LOG_WRN("State changed: WARNING");
    } else {
        LOG_INF("State changed: NORMAL");
    }

    previous_state = state;
}        }
    }
}
int main(void)
{
    LOG_INF("Sensor monitoring application started");
    return 0;
}
K_THREAD_DEFINE(producer_tid, PRODUCER_STACK_SIZE, producer_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(consumer_tid, PRODUCER_STACK_SIZE, consumer_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
/*
K_THREAD_DEFINE(name, stack_size, entry,
                p1, p2, p3, priority, options, delay);
pi, p2, p3 -> argument passed into the thread
options -> sometimes options can be used to select things like user mode
*/
