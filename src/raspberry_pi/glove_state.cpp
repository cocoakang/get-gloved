#include"glove_state.h"
#include <assert.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

int glove_state::to_bytes(char* buff,int buffer_size)
{
    const int float_per_imu = 4;
    int total_len = 0;
    for(int which_imu = 0; which_imu < this->imu_num; which_imu++){
        int char_num = sizeof(char)*4*float_per_imu;
        assert(buffer_size > 0);
        memcpy(buff,(char*)(this->imu_states[which_imu].quats),char_num);
        buff+=char_num;
        buffer_size-=char_num;
        total_len+=char_num;
    }
    return total_len;
}

void glove_state::init(const int imu_num)
{
    this->imu_num = imu_num;
    this->imu_states.resize(this->imu_num);
    for(int which_imu = 0; which_imu < imu_num; which_imu++){
        clock_gettime(CLOCK_REALTIME, &(imu_states[which_imu].time_stamp));
    }
}