#ifndef CONFIG_H_
#define CONFIG_H_

/* Dimensions */
#define N_STEPPERS 4
#define N_SENSORS 7
#define N_CONTROLLERS 4
#define N_FUNCTIONS (N_STEPPERS + N_SENSORS + N_CONTROLLERS)

/* General settings */
#define CONTROLLER_REFRESH_PERIOD 20 /* 50 Hertz */
#define CONTROLLER_INTEGRAL_ENABLE FALSE /* No integral term for PID */
#define CONTROLLER_FAST_MULTIPLIER 10.0f
#define SDADC_AVERAGE_FACTOR 32U
#define MODBUS_FRAME_LENGTH 128U /* Max frame size for modbus server */
#define USART_BAUDRATE 115200U /* Baud rate for serial communication */

/* Volumeter coefficients */
#define VOLUM_CTRL_KP 0.008f
#define VOLUM_CTRL_KD 0.2f
#define VOLUM_CLAMP_SPEED  300.0f
#define VOLUM_ERROR_MARGIN 3000.0f
#define VOLUM_ACCEL_FREE   10000.0f
#define VOLUM_ACCEL_LOCKED 3000.0f


/* Load frame coefficients */
#define LOAD_CTRL_KP 30.0f
#define LOAD_CTRL_KD 3.0f
#define LOAD_CLAMP_SPEED  13000.0f
#define LOAD_ERROR_MARGIN 10.0f
#define LOAD_ACCEL_FREE   30000.0f
#define LOAD_ACCEL_LOCKED 30000.0f

/* Shear device specific coefficients */
#define SHEAR_CTRL_KP 10.0f
#define SHEAR_CTRL_KD 30.0f

#endif /* CONFIG_H_ */
