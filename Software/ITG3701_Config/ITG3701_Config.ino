#include <Wire.h>   

// See also ITG3701 Register Map and Descriptions
// http://www.invensense.com/mems/gyro/documents/RM-000001-ITG-3701-RM.pdf
//
////////////////////////////
// ITG3701 Gyro Registers //
////////////////////////////
//#define ITG3701_ZG_OFFS_USRH     0x17
//#define ITG3701_ZG_OFFS_USRL     0x18
//#define ITG3701_SMPLRT_DIV       0x19
#define ITG3701_CONFIG           0x1A
#define ITG3701_GYRO_CONFIG      0x1B
//#define ITG3701_FIFO_EN          0x23
#define ITG3701_INT_PIN_CFG      0x37
#define ITG3701_INT_ENABLE       0x38
//#define ITG3701_INT_STATUS       0x3A
//#define ITG3701_GYRO_XOUT_H      0x43
//#define ITG3701_GYRO_XOUT_L      0x44
//#define ITG3701_GYRO_YOUT_H      0x45
//#define ITG3701_GYRO_YOUT_L      0x46
#define ITG3701_GYRO_ZOUT_H      0x47
#define ITG3701_GYRO_ZOUT_L      0x48
#define ITG3701_USER_CTRL        0x6A  
#define ITG3701_PWR_MGMT_1       0x6B // Device defaults to the SLEEP mode
#define ITG3701_PWR_MGMT_2       0x6C
//#define ITG3701_FIFO_COUNTH      0x72
//#define ITG3701_FIFO_COUNTL      0x73
//#define ITG3701_FIFO_R_W         0x74
#define ITG3701_WHO_AM_I         0x75 // Should return 0x68

#define ITG3701_ADDRESS 0x68

int calVal;  // Gyroscope DC Offset Calibration Value 

void setup()
{ Wire.begin();
  Serial.begin(9600);
  initGyro();
}

void loop()
{ 
  Serial.println(readGyro());
  delay(300);
  
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
