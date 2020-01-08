


#include "imu.h"

//#ifdef MPU9250
MPU9250 myIMU;
//#endif

void imu_sleep()
{
  uint8_t c = 0;

  // Disable gyro and accel
  c = myIMU.readByte(MPU9250_ADDRESS, PWR_MGMT_2);
  c |= 0x3F;    // Set sleep bit [6]
  myIMU.writeByte(MPU9250_ADDRESS, PWR_MGMT_2, c);
  delay(100);

  // Enter sleep mode
  c = myIMU.readByte(MPU9250_ADDRESS, PWR_MGMT_1);
  c |= 0x40;    // Set sleep bit [6]
  myIMU.writeByte(MPU9250_ADDRESS, PWR_MGMT_1, c);
  delay(100);


  // Disable magnetometer
  c = myIMU.readByte(AK8963_ADDRESS, AK8963_CNTL);
  myIMU.writeByte(AK8963_ADDRESS, AK8963_CNTL, c & ~(0x0F) ); // Clear bits 0 - 3 to power down magnetometer
  
  //c = myIMU.readByte(MPU9250_ADDRESS, PWR_MGMT_1);
  //myIMU.writeByte(MPU9250_ADDRESS, PWR_MGMT_1, c | 0x10); // Write bit 4 to enable gyro standby delay(10); // Wait for all registers to reset
}

void imu_wake()
{
  myIMU.begin();
  init_imu();
}

//#ifdef MPU9250
void init_imu()
{
    // Read the WHO_AM_I register, this is a good test of communication
  Serial.println("Attempting to read WHO_AM_I_MPU9250...");
  byte c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
  Serial.print(F("MPU9250 I AM 0x"));
  Serial.print(c, HEX);
  Serial.print(F(" I should be 0x"));
  Serial.println(0x71, HEX);

  if (c == 0x71) // WHO_AM_I should always be 0x71
  {
    Serial.println(F("MPU9250 is online..."));

    // Start by performing self test and reporting values
    myIMU.MPU9250SelfTest(myIMU.selfTest);
    Serial.print(F("x-axis self test: acceleration trim within : "));
    Serial.print(myIMU.selfTest[0],1); Serial.println("% of factory value");
    Serial.print(F("y-axis self test: acceleration trim within : "));
    Serial.print(myIMU.selfTest[1],1); Serial.println("% of factory value");
    Serial.print(F("z-axis self test: acceleration trim within : "));
    Serial.print(myIMU.selfTest[2],1); Serial.println("% of factory value");
    Serial.print(F("x-axis self test: gyration trim within : "));
    Serial.print(myIMU.selfTest[3],1); Serial.println("% of factory value");
    Serial.print(F("y-axis self test: gyration trim within : "));
    Serial.print(myIMU.selfTest[4],1); Serial.println("% of factory value");
    Serial.print(F("z-axis self test: gyration trim within : "));
    Serial.print(myIMU.selfTest[5],1); Serial.println("% of factory value");

    // Calibrate gyro and accelerometers, load biases in bias registers
    myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);

    myIMU.initMPU9250();
    // Initialize device for active mode read of acclerometer, gyroscope, and
    // temperature
    Serial.println("MPU9250 initialized for active data mode....");


    // Read the WHO_AM_I register of the magnetometer, this is a good test of
    // communication
    byte d = myIMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
    Serial.print("AK8963 ");
    Serial.print("I AM 0x");
    Serial.print(d, HEX);
    Serial.print(" I should be 0x");
    Serial.println(0x48, HEX);

    if (d != 0x48)
    {
      // Communication failed, stop here
      Serial.println(F("Communication failed, abort!"));
      Serial.flush();
      abort();
    }

    // Get magnetometer calibration from AK8963 ROM
    myIMU.initAK8963(myIMU.factoryMagCalibration);
    // Initialize device for active mode read of magnetometer
    Serial.println("AK8963 initialized for active data mode....");

    

    // Get sensor resolutions, only need to do this once
    myIMU.getAres();
    myIMU.getGres();
    myIMU.getMres();


    /*
    // The next call delays for 4 seconds, and then records about 15 seconds of
    // data to calculate bias and scale.
    myIMU.magCalMPU9250(myIMU.magBias, myIMU.magScale);
    Serial.println("AK8963 mag biases (mG)");
    Serial.println(myIMU.magBias[0]);
    Serial.println(myIMU.magBias[1]);
    Serial.println(myIMU.magBias[2]);

    Serial.println("AK8963 mag scale (mG)");
    Serial.println(myIMU.magScale[0]);
    Serial.println(myIMU.magScale[1]);
    Serial.println(myIMU.magScale[2]);
    delay(2000L); // Add delay to see results before serial spew of data
    */
    
    Serial.flush();
  }
  //delay(5);
}

void update_imu()
{
    // If intPin goes high, all data registers have new data
  // On interrupt, check if data ready interrupt
  if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {
    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values

    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    myIMU.ax = (float)myIMU.accelCount[0] * myIMU.aRes; // - myIMU.accelBias[0];
    myIMU.ay = (float)myIMU.accelCount[1] * myIMU.aRes; // - myIMU.accelBias[1];
    myIMU.az = (float)myIMU.accelCount[2] * myIMU.aRes; // - myIMU.accelBias[2];

    myIMU.readGyroData(myIMU.gyroCount);  // Read the x/y/z adc values

    // Calculate the gyro value into actual degrees per second
    // This depends on scale being set
    myIMU.gx = (float)myIMU.gyroCount[0] * myIMU.gRes;
    myIMU.gy = (float)myIMU.gyroCount[1] * myIMU.gRes;
    myIMU.gz = (float)myIMU.gyroCount[2] * myIMU.gRes;

    myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values

    // Calculate the magnetometer values in milliGauss
    // Include factory calibration per data sheet and user environmental
    // corrections
    // Get actual magnetometer value, this depends on scale being set
    myIMU.mx = (float)myIMU.magCount[0] * myIMU.mRes
               * myIMU.factoryMagCalibration[0] - myIMU.magBias[0];
    myIMU.my = (float)myIMU.magCount[1] * myIMU.mRes
               * myIMU.factoryMagCalibration[1] - myIMU.magBias[1];
    myIMU.mz = (float)myIMU.magCount[2] * myIMU.mRes
               * myIMU.factoryMagCalibration[2] - myIMU.magBias[2];
  } // if (readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
}

void print_mag(){
  // Print mag values in degree/sec
  Serial.print("X-mag field: "); Serial.print(myIMU.mx);
  Serial.print(" mG ");
  Serial.print("Y-mag field: "); Serial.print(myIMU.my);
  Serial.print(" mG ");
  Serial.print("Z-mag field: "); Serial.print(myIMU.mz);
  Serial.println(" mG");
}

float get_heading(){
  
}
//#endif
