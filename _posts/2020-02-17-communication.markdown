---
layout: post
title: Communication
tags: [frontpage, jekyll, blog]
image: '/images/posts/wifi.png'
---

At this point, we can get gesture from MPU6050, but how to we use that? This project aims to make a data glove, so the data will be processed in a VR game which runs on a mobile phone or PC. Bluetooth is the best way to communicate between PC and other small setups, data glove in our case. However, using Bluetooth in Unity3d on Windows isn't trivial. Meanwhile, I wrote a class using UDP to transport data in another project. So I choose udp transportation to communicate between the VR game program and data collecting program running on Raspberry Pi. If you have any good method to leverage Bluetooth in Unity3d, sharing that is welcomed!  

### UDP with linux
Creating socket and using that for UDP transport on Raspberry Pi isn't hard. I recommend this [site](https://www.binarytides.com/programming-udp-sockets-c-linux/) to learn how to using UDP.  
It worth noting that UDP server should be created as a daemon thread so that the gesture updating thread can running without delay. A lock is needed also to manage threads concurrent reading. On linux platform, pthread library is pretty easy to use manage multithreading programming. C++ 11 thread library is an alternative. If you are not familiar with that, there are tremendous blogs talk about that.  
In this project, the class for transport is named dragon. You can find in /src/raspberry_pi/dragon.h

### UDP with Unity3d
Our VR games is a UDP client, which acquires hand gesture from Raspberry Pi. Using UDP in Unity3d is almost as same as UDP in linux. You just need create a C# script and use Unity3d built in UDP class. This [site](https://forum.unity.com/threads/simple-udp-implementation-send-read-via-mono-c.15900/) is useful to learn how to play with that.

