# Zephyr RTOS Sensor Monitoring System

A real-time embedded sensor monitoring project built with **Zephyr RTOS** on an **STM32 Nucleo-F401RE** board.

This project integrates physical **BME280** and **MPU6050** sensors over I2C, uses Zephyr RTOS threads and message queues for producer-consumer communication, and classifies system health into `NORMAL`, `WARNING`, and `FAULT` states based on live temperature and vibration readings.

## Overview

This application implements a real-time sensor monitoring pipeline using physical hardware.

A producer thread periodically reads sensor data from:

- **BME280** temperature sensor
- **MPU6050** accelerometer

The data is packaged into a structured message and sent through a Zephyr `k_msgq`. A consumer thread receives the message, logs the sensor values, and evaluates the system state based on configurable thresholds.

The project originally began as a simulation-based RTOS pipeline and was later extended to use real I2C sensors.

## Features

- Real-time sensor acquisition using Zephyr RTOS
- BME280 temperature sensor integration over I2C
- MPU6050 accelerometer integration over I2C
- Producer-consumer thread architecture
- Zephyr `k_msgq` based inter-thread communication
- Threshold-based system health classification
- State transition logging for `NORMAL`, `WARNING`, and `FAULT`
- Hardware-backed validation on STM32 Nucleo-F401RE
- Designed for future CAN bus communication

## Hardware Used

- STM32 Nucleo-F401RE
- BME280 temperature/pressure/humidity sensor module
- MPU6050 accelerometer/gyroscope module
- Breadboard and jumper wires

## Technologies Used

- Zephyr RTOS
- Embedded C
- STM32 Nucleo-F401RE
- I2C
- Zephyr Sensor API
- Zephyr kernel threads
- Zephyr message queues
- West build system
- CMake
- VS Code

## System Architecture

```text
BME280 + MPU6050
        |
        | I2C
        v
Producer Thread
        |
        | k_msgq
        v
Consumer Thread
        |
        v
State Classification
NORMAL / WARNING / FAULT
```

## Thread Design
## Producer Thread
The producer thread runs periodically every 1 second. It reads live sensor data from the BME280 and MPU6050, converts the values into a structured format, derives a simple vibration metric from accelerometer readings, and pushes the result into a Zephyr message queue.

## Consumer Thread
The consumer thread waits on the message queue, receives sensor packets, logs the readings, and classifies the system state based on temperature and vibration thresholds.

## Sensor Data Structure
struct sensor_data {
    int temperature_c;
    int accel_x_milli;
    int accel_y_milli;
    int accel_z_milli;
    int vibration_milli;
};

## Example output 
*** Booting Zephyr OS build v4.4.0-rc1-178-gccfd5efa09f9 ***
[00:00:00.014,000] <inf> app: Sensor monitoring application started
[00:00:01.014,000] <inf> app: Producer: sensors ready
[00:00:02.021,000] <inf> app: Consumer: temp=23 C vib=14965 accel=(1762,6153,-7050)
[00:00:03.024,000] <inf> app: Consumer: temp=23 C vib=14875 accel=(1649,6200,-7026)
[00:00:41.191,000] <wrn> app: State changed: WARNING
[00:00:43.197,000] <err> app: State changed: FAULT
[00:00:44.201,000] <inf> app: State changed: NORMAL

<img width="1147" height="286" alt="image" src="https://github.com/user-attachments/assets/0c0239a2-2b35-473e-846c-5153d029b472" />

## Current Status
Implemented and tested:

- I2C communication with both sensors
- BME280 temperature readings
- MPU6050 accelerometer readings
- RTOS producer-consumer pipeline
- Message queue communication
- Runtime state classification
- State transition logging

## Future Improvements
- Add CAN bus transmission using external CAN modules
- Send sensor status frames to another CAN node
- Add watchdog/error recovery
- Improve vibration detection using filtering or moving averages
- Add configurable thresholds
- Add unit tests for state classification logic
- Extend project into a distributed sensor ECU prototype

## Project Structure
```text
.
├── src/
│   ├── main.c
│   └── sensor_data.h
├── app.overlay
├── prj.conf
├── CMakeLists.txt
└── README.md
```
## Build and Flash
python -m west build -b nucleo_f401re -s . -p always
python -m west flash

## Serial Monitor
python -m serial.tools.miniterm COM3 115200
Press the RESET button on the Nucleo board after opening the serial monitor.


