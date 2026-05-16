# ESP32 micro-ROS controller

A dual-node embedded communication system bridging wireless ESP-NOW input devices with a micro-ROS stack for mobile robotics applications.

The system connects a handheld controller and a robot-side ESP32 interface into a low-latency telemetry and control pipeline integrated with ROS 2 (Jetson Nano).

---

## Project Overview

The system consists of two cooperating ESP32 firmware nodes:

- a wireless handheld controller for operator input and telemetry,
- a robot-side embedded interface acting as an ESP-NOW receiver and micro-ROS bridge.

Together, they form a real-time communication layer between physical operator control, onboard robot sensors, and higher-level ROS 2 logic.

---

## System Architecture

### High-level system overview

![System overview (robot + controller)](images/main.jpg)

The architecture is split into two independent but cooperating ESP32 firmware applications communicating via ESP-NOW and ROS 2.

---

## Controller Node

![Handheld controller](images/controller.jpg)

The controller-side ESP32 acts as a wireless handheld input device.

### Responsibilities

- joystick ADC acquisition (X / Y axes)
- push-button state monitoring
- battery voltage measurement
- ADC compensation relative to battery level
- ESP-NOW packet transmission over fixed Wi-Fi channel

### Output

The controller continuously broadcasts normalized operator input packets to the robot-side node.

---

## Robot Interface Node

![Robot interface / ESP32 mounted on robot](images/front.jpg)

The robot-side ESP32 serves as the main embedded I/O bridge between physical robot hardware and the ROS 2 ecosystem.

### Responsibilities

- receiving wireless controller packets via ESP-NOW
- reading local analog sensor arrays (ADC)
- publishing telemetry via micro-ROS topics
- receiving ROS 2 commands from Jetson Nano
- controlling buzzer output

This node combines wireless operator input, local sensing, and bidirectional ROS 2 communication inside a single FreeRTOS-based firmware application.

---

## Implemented Functionality

### Wireless operator telemetry

The system transmits:

- joystick X axis
- joystick Y axis
- push-button state
- controller battery voltage

Data is sent via ESP-NOW and forwarded into ROS 2 topics by the robot-side ESP32.

---

### Local robot sensor acquisition

The robot-side ESP32 performs continuous ADC acquisition from onboard sensors.

Current configuration includes:

- IR line detection sensors
- photoresistor-based light sensors

Sensor values are periodically published to ROS 2 for higher-level processing.

---

### ROS 2 command interface

The robot-side firmware subscribes to ROS 2 control topics.

Currently implemented:

- buzzer activation command

This enables bidirectional communication between ROS 2 logic and embedded hardware.

---

## Firmware Design

Both ESP32 devices run independent FreeRTOS-based firmware applications.

The architecture emphasizes:

- concurrent task execution
- low-latency ESP-NOW communication
- periodic ADC sampling
- non-blocking micro-ROS integration
- modular separation of controller and robot responsibilities

This design allows the system to scale into larger robotic communication architectures without redesign.

---

## ADC Battery Compensation

Because the controller is battery-powered, joystick ADC readings are affected by voltage drift.

To mitigate this, the controller continuously measures battery voltage and rescales joystick values back into a normalized 12-bit range before transmission.

This provides:

- stable operator control
- reduced dependency on battery state
- consistent ROS 2 telemetry input

---

## ROS 2 Topics

### Published topics

```text
/esp/sensors     # local robot analog sensor values
/esp/joystick    # wireless controller joystick telemetry
/esp/button      # wireless controller push-button state
/esp/battery     # controller battery voltage [mV]
```

### Subscribed topics

```text
/esp/horn        # buzzer activation command
```

---

### Youtube videos

https://www.youtube.com/playlist?list=PLLbGYqHAyf1eH0HuIkOklTwuBiebvZPPm
