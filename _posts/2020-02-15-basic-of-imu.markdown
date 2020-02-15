---
layout: post
title: Basic of IMU
tags: [frontpage, jekyll, blog]
image: '/images/posts/imu.jpg'
---

## Introduction

[IMU](https://en.wikipedia.org/wiki/Inertial_measurement_unit)(Inertial measurement unit) is a kind of sensor to measure the acceleration, rotationrate or orientation of the device. There are a lot of types off the shelf in the market, and [MPU6050](https://www.invensense.com/products/motion-tracking/6-axis/mpu-6050/) is one of them.

I don't want to describe all details about how to control a sensor like that, since there are so many blogs talk about that. What I am going to focus on is the pitfalls and something else you can not easily found on Internet. If you have no knowledge about how to contorl MPU6050, I recommend you to read this [blog](https://tutorials-raspberrypi.com/measuring-rotation-and-acceleration-raspberry-pi/).

## Contents
1. Communicate in python or c++
2. Circuit pitfalls

## Communicate in python or c++
Two ways can be used to access MPU6050, [I2C](https://i2c.info/) or [SPI](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface). I2C is used in this project since most of libraries using this which means you can find more resources.  
  
So how to access MPU6050 using I2C on Raspberry Pi? We need to choose a language. Python or C++, specifically.  
  
This project is going to make a data glove, so real-time response is the biggest thing. And there is a fantastic open source library named [i2cdevlib](https://github.com/jrowberg/i2cdevlib) written in C++, which gives you so many examples about how to control MPU6050. So C++ wins. :D

In i2cdevlib, there is an [example file](https://github.com/jrowberg/i2cdevlib/blob/master/RaspberryPi_bcm2835/MPU6050/examples/MPU6050_example_1.cpp) for Raspberry Pi to control MPU6050. However, it includes arduino's implementation, MPU6050.h and MPU6050.cpp, specifically. In those arduino files, some AVR built-in functions are called, which let the compiler complain. I adjust the arduino files for Raspberry pi, which you can find in src/raspberry_pi/ .

It worth noting that the example file ask for bcm2835 library. Help of install bcm2835 can be found [here](https://www.airspayce.com/mikem/bcm2835/).

## Circuit pitfalls
### About I2C and address
In the example cpp codes, MPU6050 address is set to 0x68 as default. So you may leave the AD0 pin float or connect it with ground. If you cannot find the device, using [i2cdetect tool](https://linux.die.net/man/8/i2cdetect) to detect this device.  
On Raspberry Pi 4B, there are two i2c bus. Bus1 is exposed to user. I heared that under Raspberry Pi 3 or lower version, Bus0 should be used.
  
  cannot open file /dev/i2c-1

When you get this error, you may change the i2c bus in I2Cdev.cpp.

### MPU6050 VCC
MPU6050 is working in VCC3.3 environment. Plug VCC5 to MPU6050 is dangerous.