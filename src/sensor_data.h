#ifndef SENSOR_SIM_H
#define SENSOR_SIM_H

struct sensor_data {
    int temperature_c;
    int accel_x_milli;
    int accel_y_milli;
    int accel_z_milli;
    int vibration_milli;
};

#endif