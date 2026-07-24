#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>

/* These signs preserve the pre-refactor behavior. Verify them with wheels raised. */
#define BOARD_MOTOR_LEFT_DIRECTION_SIGN   (+1)
#define BOARD_MOTOR_RIGHT_DIRECTION_SIGN  (+1)
#define BOARD_ENCODER_LEFT_SIGN           (+1)
#define BOARD_ENCODER_RIGHT_SIGN          (-1)
#define BOARD_MOTOR_LEFT_FORWARD_A_HIGH    1
#define BOARD_MOTOR_RIGHT_FORWARD_A_HIGH   0

/* The installed MPU6050 orientation must be verified on the assembled car. */
#define BOARD_IMU_PITCH_SIGN              (+1.0f)
#define BOARD_IMU_ROLL_SIGN               (+1.0f)
#define BOARD_IMU_YAW_SIGN                (+1.0f)
#define BOARD_IMU_GYRO_X_SIGN             (+1.0f)
#define BOARD_IMU_GYRO_Y_SIGN             (+1.0f)
#define BOARD_IMU_GYRO_Z_SIGN             (+1.0f)
#define BOARD_IMU_ORIENTATION_MATRIX       { 1, 0, 0, 0, 1, 0, 0, 0, 1 }

#define BOARD_OLED_SEGMENT_REMAP_COMMAND  0xA1U
#define BOARD_OLED_COM_SCAN_COMMAND        0xC8U

#define BOARD_MOTOR_PWM_PERIOD            7199U
#define BOARD_MOTOR_PWM_PRESCALER         0U
/* TIM1 runs at 72 MHz on the current clock tree: 72 MHz / (7199 + 1) = 10 kHz. */
#define BOARD_MOTOR_PWM_FREQUENCY_HZ      10000U

#define BOARD_USART3_BAUD                 9600U

#endif
