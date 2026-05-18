# ESP32 micro-ROS Controller

## Overview

Dual ESP32 system combining ESP-NOW wireless communication with micro-ROS for ROS 2 integration.

One ESP32 works as a handheld controller, the second acts as a robot-side bridge to ROS 2 running on a Jetson Nano.

The system enables low-latency wireless control, sensor reading, and ROS 2 topic communication.

## Tech Stack

- ESP32
- Embedded C with ESP-IDF
- FreeRTOS
- micro-ROS
- ADC

## Gallery

### System overview
![System overview](images/main.jpg)

### Handheld controller
![Controller](images/controller.jpg)

### Robot interface
![Robot interface](images/front.jpg)

### Videos

https://www.youtube.com/playlist?list=PLLbGYqHAyf1eH0HuIkOklTwuBiebvZPPm
