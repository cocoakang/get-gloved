---
layout: post
title: Get More IMUS
tags: [frontpage, jekyll, blog]
image: '/images/posts/tca.jpg'
---

The possible I2C address of MPU6050 is 0x68 or 0x69, depending on AD0 is connected to GND or VCC. Measuring hand gesture with only two IMUs is impossible. Don't worry, there is a I2C bus address multiplexer named TCA9548 can save us.  
The details can be found [here](https://learn.adafruit.com/adafruit-tca9548a-1-to-8-i2c-multiplexer-breakout/overview).  

### Hardwares
With all preparation, we can finally measure hand gesture now. Each finger needs one IMU to measure its gesture and another IMU is needed for sensing gesture of the whole hand. So at least 6 IMUs are acquired. Unfortunately, I only have 5 IMUs and cannot buy alternative due to the dicease SARS-CoV. So the pinky is abandoned.  
  
So the final glove looks like this:
![HARDWARE](/get-gloved/images/posts/hardware.jpg)  
  
The VCC, GND, I2C bus pins are connected to a breadboard. Other IMUs are connected to breadboard using Dupont Lines.  
  
The logic graph is :
![LOGIC](/get-gloved/images/posts/logic.png)  
  
### Softwares
Whole program on Rapberry pi can be found in /src/raspberry_pi/ .  To compile it, just move to this folder and make.