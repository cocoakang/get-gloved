#ifndef __GLOVE_H__
#define __GLOVE_H__

#include<cmath>
#include<string>
#include <stdint.h>
#include<stdio.h>
#include<vector>
#include"glove_state.h"
//#include "MPU6050_6Axis_MotionApps20.hpp"
#define MPU6050_INCLUDE_DMP_MOTIONAPPS20
#include"MPU6050.h"
#include <unistd.h>
#include<pthread.h>

#define OUTPUT_RAW_MOTION
#define OUTPUT_YPR
//#define OUTPUT_READABLE_YAWPITCHROLL
// #define OUTPUT_READABLE_WORLDACCEL
// #define OUTPUT_READABLE_REALACCEL
//#define OUTPUT_READABLE_EULER

#define _BV(bit) (1 << (bit))

class glove
{
private:
    std::vector<bool> converged;
    pthread_rwlock_t glove_state_rwlock;
    int imu_num;
    std::vector<MPU6050> theSensors;
    std::vector<uint16_t> packetSizes;
public:
    glove(int imu_num,std::vector<MPU6050> theSensors,std::vector<uint16_t> packetSizes);
    ~glove();
    int update_state(const int which_imu,FILE* pf_output);
    pthread_rwlock_t* get_rwlock();
    glove_state* get_glove_state_ptr();

    glove_state the_state;
};




#endif