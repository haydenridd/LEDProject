#include <Wire.h> 
#include <Adafruit_L3GD20.h>

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

Adafruit_L3GD20 gyro;

long sum;
int accel_in = 0;
int masked_in;

void setup()
{
  pinMode(r,OUTPUT);
  pinMode(g,OUTPUT);
  pinMode(b,OUTPUT);  
  
  if (!gyro.begin(gyro.L3DS20_RANGE_2000DPS))
  {
    while (1);
  }
  
  //analogWrite(r,1);
  //analogWrite(g,1);
}

void loop()
{
  sum = 0;
  for(int i = 0; i < 8; i++)
  { gyro.read();
    sum = sum + abs(gyro.data.z);
    delay(1);
  }
  sum = sum >> 3;
  accel_in = (int)sum;
  
  // assuming accel_in = signed 16 bit input
  // r -> pwm value for red from 0 -> 255
  // g -> pwm value for green 0 -> 255
  // b -> pwm value for blue 0 -> 255
  
//  accel_in = accel_in + 1;

  
//  for(int i = 0; i < 127; i++)
//  { analogWrite(g,127-i);
//    analogWrite(r,i);
//      delay(10);
//  }
  
accel_in = accel_in & ABS_MASK; // take absolute value of accelerometer data
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

