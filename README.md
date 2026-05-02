# ESP32 micro-ROS control bridge

An embedded dual-node communication and telemetry system integrating wireless ESP-NOW data transfer with micro-ROS topic publication for ROS 2 mobile robotics platforms.

The project consists of two cooperating ESP32-based firmware nodes:

* a handheld wireless controller responsible for joystick, button and battery acquisition,
* a robot-side embedded interface responsible for local sensor acquisition, wireless packet receiving and ROS 2 communication.

Together, the system provides a low-latency bridge between physical operator input, robot-mounted analog sensors and the higher-level ROS 2 software stack running on Jetson Nano.

---

## System Architecture

The firmware is split into two independent but cooperating ESP32 applications.

### Controller Node

The controller-side ESP32 acts as a wireless handheld input device.

Its responsibilities include:

* joystick ADC acquisition (X/Y axes),
* push-button state monitoring,
* battery voltage measurement,
* ADC compensation relative to battery level,
* ESP-NOW packet transmission over a fixed Wi-Fi channel.

The controller continuously broadcasts normalized operator input packets to the robot-side node.

---

### Robot Interface Node

The robot-side ESP32 serves as the main embedded I/O bridge between the physical robot hardware and the ROS 2 environment.

Its responsibilities include:

* receiving wireless controller packets through ESP-NOW,
* reading local analog sensor arrays,
* publishing sensor and controller telemetry through micro-ROS topics,
* receiving ROS 2 commands from Jetson Nano,
* controlling buzzer output based on subscribed horn commands.

This node combines wireless operator input, local physical sensing and bidirectional ROS 2 communication inside a single FreeRTOS-based firmware application.

---

## Implemented Functionality

### Wireless joystick and operator telemetry

The system wirelessly transfers:

* joystick X axis,
* joystick Y axis,
* controller push-button state,
* controller battery voltage.

Data is transmitted through ESP-NOW and republished to ROS 2 topics by the robot-side ESP32.

---

### Local robot sensor acquisition

The robot-side ESP32 performs ADC acquisition from onboard analog sensors.

Current hardware configuration includes:

* IR line detection sensors,
* photoresistor-based light sensors.

Sensor values are periodically published to ROS 2 for higher-level decision making on the Jetson Nano.

---

### ROS 2 command reception

The robot-side firmware subscribes to external ROS 2 command topics.

Currently implemented:

* buzzer activation command.

This creates a bidirectional communication bridge where the ESP32 not only publishes physical telemetry, but also receives actuator-level control instructions from the ROS 2 layer.

---

## Firmware Design

Both ESP32 devices run independent FreeRTOS-based firmware applications.

The design emphasizes:

* concurrent task execution,
* low-latency wireless packet handling,
* periodic ADC acquisition,
* non-blocking ROS communication,
* modular separation between controller and robot-side responsibilities.

This allows the system to scale into a larger robotic communication subsystem without redesigning the firmware architecture.

---

## ADC Battery Compensation

The handheld controller is battery-powered, which introduces joystick ADC drift as supply voltage changes.

To reduce this effect, the controller continuously measures battery voltage and rescales joystick ADC values back into a normalized 12-bit measurement range before transmission.

This provides:

* more stable operator control,
* reduced dependency on battery discharge level,
* direct compatibility with the rest of the ADC-based ROS telemetry.

---

## Repository Structure

```text
/src
  /micro_ros     # Robot-side ESP32 firmware: ESP-NOW RX, ADC sensors, micro-ROS bridge
  /controller    # Wireless handheld controller: ADC acquisition, battery compensation, ESP-NOW TX
```

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

## Known Constraints

* ESP-NOW requires both ESP32 devices to remain on the same Wi-Fi channel.
* Joystick center calibration depends on ADC tolerance and mechanical joystick neutral position.
* Sensor array count is expandable as additional ADC channels are integrated.

---

## Future Development

* expansion from current sensor set to full robot sensor array,
* enclosure preparation for handheld controller,
* demonstration recordings and ROS-side visualization.
