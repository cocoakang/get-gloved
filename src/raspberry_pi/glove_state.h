#ifndef __GLOVE_STATE_H__
#define __GLOVE_STATE_H__

#include<vector>
#include<unistd.h>
#include<time.h>

struct imu_state
{
    float angles[3] = {0.0,0.0,0.0};
    float position[3] = {0.0,0.0,0.0};
    float velocity[3] = {0.0,0.0,0.0};
    float quats[4] = {0.0,0.0,0.0,0.0};
    struct timespec time_stamp;
};

class glove_state
{
public:
    int imu_num;
    std::vector<struct imu_state> imu_states;
    void init(const int imu_num);
    int to_bytes(char* buff,int buffer_size);
};

#endif