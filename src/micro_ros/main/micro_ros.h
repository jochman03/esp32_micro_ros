/*
 *  micro_ros.h
 *
 *  Created on: 1 May 2026
 *  Author: jochman03
 */

#ifndef MAIN_MICRO_ROS_H
#define MAIN_MICRO_ROS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize micro-ROS and start communication task.
 *
 * Configures required GPIOs and creates micro-ROS FreeRTOS task.
 */
void micro_ros_init(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_MICRO_ROS_H */