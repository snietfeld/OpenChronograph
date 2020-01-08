#ifndef __IMU_H__
#define __IMU_H__

#include "thirdparty/SparkFun_MPU-9250_Breakout_Arduino_Library/src/MPU9250.h"

  
extern MPU9250 myIMU;
  

void init_imu(void);
void update_imu(void);
void imu_sleep(void);
void imu_wake(void);

#endif // __IMU_H__
