#ifndef __DRAGON_H__
#define __DRAGON_H__

#include"thread.h"
#include"glove_state.h"
#include<pthread.h>

#define       PT_CHECK_CONNECT           0x1
#define       PT_CHECK_CONNECT_ACK       0x2	
#define       PT_GRAB_GLOVE_STATE        0x3    	

class Dragon:public Thread
{
private:
    bool should_landing;
    int sock;
    int port;
    pthread_rwlock_t* rwlock;
    glove_state* p_state;
    glove_state glove_state_reg;
public:
    Dragon(int port,pthread_rwlock_t* rwlock,glove_state* p_state);
    int init_net();
    int hover();
    void Run();
    ~Dragon();
    void landing();
private:
    void glance_glove_state();
};

#endif