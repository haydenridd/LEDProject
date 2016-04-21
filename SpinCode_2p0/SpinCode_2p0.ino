#include <Wire.h> 

// Masking Defs
#define BIT_MASK 8191 // 13 bits, all ones
#define EIGHT_BIT_MAX 255 // 8 bits, all ones, the maximum value the LED would take (255), used for flipping ranges 
#define ABS_MASK 32767 // 15 bits, all ones, && with 16 bit signed integer to take off sign bit
#define QUARTER ABS_MASK >> 2 // 15 bits all ones divided by 4
#define HALF ABS_MASK >> 1 // 15 bits all ones divided by 2
#define THREE_QUARTER QUARTER + HALF // summed together to produce 3/4
#define SHIFT_NUM 6 // right shifting the masked accelerometer data by this number will change the range to 0 -> 127

#define r 10
#define g 12
#define b 9

#define ITG3701_CONFIG           0x1A
#define ITG3701_GYRO_CONFIG      0x1B
#define ITG3701_INT_PIN_CFG      0x37
#define ITG3701_INT_ENABLE       0x38
#define ITG3701_GYRO_ZOUT_H      0x47
#define ITG3701_GYRO_ZOUT_L      0x48
#define ITG3701_USER_CTRL        0x6A  
#define ITG3701_PWR_MGMT_1       0x6B // Device defaults to the SLEEP mode
#define ITG3701_PWR_MGMT_2       0x6C
#define ITG3701_WHO_AM_I         0x75 // Should return 0x68
#define ITG3701_ADDRESS 0x68

long sum;
int accel_in = 0;
int masked_in;
int calVal;  // Gyroscope DC Offset Calibration Value

void setup()
{
  pinMode(r,OUTPUT);
  pinMode(g,OUTPUT);
  pinMode(b,OUTPUT);  
  Wire.begin();
  initGyro();
}

void loop()
{
  sum = 0;
  for(int i = 0; i < 8; i++)
  { sum = sum + readGyro();
    delay(1);
  }
  sum = sum >> 3;
  accel_in = (int)sum;

masked_in = accel_in & BIT_MASK; // 13 bit mask of accelerometer data
  
if(accel_in < QUARTER){ // first quarter of accelerometer range
analogWrite(r,0);
analogWrite(g,masked_in >> SHIFT_NUM); // green ranges 0 -> 127
analogWrite(b,EIGHT_BIT_MAX - ( masked_in >> SHIFT_NUM)); // blue ranges 255 -> 128
}
else if(accel_in < HALF){ // second quarter of accelerometer range
analogWrite(r,0);
analogWrite(g,(EIGHT_BIT_MAX >> 1) + (masked_in >> SHIFT_NUM)); // green ranges 128 -> 255
analogWrite(b,(EIGHT_BIT_MAX >> 1) - (masked_in >> SHIFT_NUM)); // blue ranges 127 -> 0
}
else if(accel_in < THREE_QUARTER){// third quarter of accelerometer range
analogWrite(r,masked_in >> SHIFT_NUM); // red ranges 0 -> 127
analogWrite(g,EIGHT_BIT_MAX - ( masked_in >> SHIFT_NUM)); // green ranges 255 -> 128
analogWrite(b,0);
}
else{ // final quarter of accelerometer range
analogWrite(r,(EIGHT_BIT_MAX >> 1) + (masked_in >> SHIFT_NUM)); // red ranges 128 -> 255
analogWrite(g,(EIGHT_BIT_MAX >> 1) - (masked_in >> SHIFT_NUM)); // green ranges 127 -> 0
analogWrite(b,0);
}

}


int readGyro(void)
{ 
  byte z_High = read8(ITG3701_GYRO_ZOUT_H);
  byte z_Low = read8(ITG3701_GYRO_ZOUT_L);
  
  return abs(((int)z_High << 8 | z_Low)-calVal);
}

void initGyro(void)
{ 
  write8(ITG3701_PWR_MGMT_1,0x81);  // Reset the device to put all registers back to default values
  delay(100);
  write8(ITG3701_PWR_MGMT_1,0x09);  // Disable the temp sensor and set the PLL as the clock source 
  write8(ITG3701_PWR_MGMT_2,0x06);  // Disable the x and y axis
  write8(ITG3701_INT_ENABLE,0x01);  // Enable the data ready interrupt
  write8(ITG3701_INT_PIN_CFG,0x20); // Set interrupt status bits to clear on a read of INT_STATUS (also set INT to latch active high push pull and disable F_SYNC)  
  write8(ITG3701_GYRO_CONFIG,0x18); // Set the FSR to 4000 dps
  write8(ITG3701_CONFIG,0x00);      // Set sample rate to 8kHz and the bandwidth to 250Hz (delay 0.97ms)
  delay(100);
  long sum = 0;
  for(int i = 128; i > 0; i--)  // Take 128 readings from the Gyro
  { 
    sum = sum + ((int)read8(ITG3701_GYRO_ZOUT_H) << 8 | read8(ITG3701_GYRO_ZOUT_L));
    delay(1);
  }
  sum = sum >> 7;  // Divide by 128
  calVal = sum;    // Set the global cal value to match calibration

}

byte read8(byte reg)
{ byte value;
  Wire.beginTransmission(ITG3701_ADDRESS);
  Wire.write((byte)reg);
  Wire.endTransmission();
  Wire.requestFrom(ITG3701_ADDRESS, (byte)1);
  value = Wire.read();
  Wire.endTransmission();

  return value;
}

void write8(byte reg, byte value)
{
  Wire.beginTransmission(ITG3701_ADDRESS);
  Wire.write((byte)reg);
  Wire.write(value);
  Wire.endTransmission();
}
