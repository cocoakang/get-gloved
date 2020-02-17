#include "glove.h"
#include"helper_3dmath.h"
#include<iostream>

#define RAD_TO_DEG (180.0/M_PI)
#define NANO_PER_SEC 1000000000.0

using namespace std;

glove::glove(int imu_num,std::vector<MPU6050> theSensors,std::vector<uint16_t> packetSizes):
    imu_num(imu_num),theSensors(theSensors),packetSizes(packetSizes)
{
    this->the_state.init(imu_num);
    converged.resize(imu_num,true);
    pthread_rwlockattr_t restrict_attr;
    pthread_rwlockattr_init(&restrict_attr);
    pthread_rwlockattr_setkind_np(&restrict_attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    if(pthread_rwlock_init(&glove_state_rwlock,&restrict_attr) != 0){
        cout <<"Init rwlock error"<<endl;
    };
}

glove::~glove()
{
    pthread_rwlock_destroy(&glove_state_rwlock);
}

int glove::update_state(const int which_imu,FILE* pf_output)
{
    Quaternion q;           // [w, x, y, z]         quaternion container
    VectorInt16 aa;         // [x, y, z]            accel sensor measurements
    VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
    VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
    VectorFloat gravity;    // [x, y, z]            gravity vector
    float euler[3];         // [psi, theta, phi]    Euler angle container
    float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
    uint8_t mpuIntStatus;
    uint16_t fifoCount;     // count of all bytes currently in FIFO
    uint8_t fifoBuffer[64]; // FIFO storage buffer
    ///////////////////////
    mpuIntStatus = theSensors[which_imu].getIntStatus();//theSensors[which_imu].getIntDataReadyStatus();
    bool data_ready =  mpuIntStatus & 0x2;
    if(!data_ready){
        return 1;
    }
    //cout <<"data ready!"<<endl;
    fifoCount = theSensors[which_imu].getFIFOCount();
    //cout <<"fifoCount:"<<fifoCount<<"PacketSize:"<<packetSizes[which_imu]<<endl;
    if(fifoCount < packetSizes[which_imu]){
        return 2;
    }
    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & _BV(MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) || fifoCount >= 1024) {
        // reset so we can continue cleanly
        theSensors[which_imu].resetFIFO();
        //fifoCount = mpu.getFIFOCount();  // will be zero after reset no need to ask
        cout << "FIFO overflow! imu id:" << which_imu<<endl;

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else{

        // read a packet from FIFO
        while(fifoCount >= packetSizes[which_imu]){ // Lets catch up to NOW, someone is using the dreaded delay()!
            theSensors[which_imu].getFIFOBytes(fifoBuffer, packetSizes[which_imu]);
            // track FIFO count here in case there is > 1 packet available
            // (this lets us immediately read more without waiting for an interrupt)
            fifoCount -= packetSizes[which_imu];
        }

        // #ifdef OUTPUT_RAW_MOTION
        //     float quat[4];
        //     quat[0] = ((fifoBuffer[0] << 8) | fifoBuffer[1]) / 16384.0f;
        //     quat[1] = ((fifoBuffer[4] << 8) | fifoBuffer[5]) / 16384.0f;
        //     quat[2] = ((fifoBuffer[8] << 8) | fifoBuffer[9]) / 16384.0f;
        //     quat[3] = ((fifoBuffer[12] << 8) | fifoBuffer[13]) / 16384.0f;
        //     for (int i = 0; i < 4; i++) if (quat[i] >= 2) quat[i] = -4 + quat[i];
        //     fwrite(quat,sizeof(float),4,pf_output);
        // #endif

        #ifdef OUTPUT_YPR
            // display Euler angles in degrees
            theSensors[which_imu].dmpGetQuaternion(&q, fifoBuffer);
            theSensors[which_imu].dmpGetGravity(&gravity, &q);
            theSensors[which_imu].dmpGetYawPitchRoll(ypr, &q, &gravity);
            
            pthread_rwlock_rdlock(&glove_state_rwlock);

            float quat[4];
            quat[0] = ((fifoBuffer[0] << 8) | fifoBuffer[1]) / 16384.0f;
            quat[1] = ((fifoBuffer[4] << 8) | fifoBuffer[5]) / 16384.0f;
            quat[2] = ((fifoBuffer[8] << 8) | fifoBuffer[9]) / 16384.0f;
            quat[3] = ((fifoBuffer[12] << 8) | fifoBuffer[13]) / 16384.0f;
            for (int i = 0; i < 4; i++) if (quat[i] >= 2) quat[i] = -4 + quat[i];
            this->the_state.imu_states[which_imu].quats[0] = quat[0];
            this->the_state.imu_states[which_imu].quats[1] = quat[1];
            this->the_state.imu_states[which_imu].quats[2] = quat[2];
            this->the_state.imu_states[which_imu].quats[3] = quat[3];

            this->the_state.imu_states[which_imu].angles[0] = ypr[0];
            this->the_state.imu_states[which_imu].angles[1] = ypr[1];
            this->the_state.imu_states[which_imu].angles[2] = ypr[2];

            struct timespec now_stamp;
            clock_gettime(CLOCK_REALTIME, &now_stamp);

            float dt = now_stamp.tv_sec + now_stamp.tv_nsec/NANO_PER_SEC - this->the_state.imu_states[which_imu].time_stamp.tv_sec - this->the_state.imu_states[which_imu].time_stamp.tv_nsec/NANO_PER_SEC;
            //cout <<dt << endl;
            if((converged[which_imu] == false)){
                if(dt > 5.0){
                    converged[which_imu] = true;
                    this->the_state.imu_states[which_imu].time_stamp = now_stamp;
                }
            }else{
                theSensors[which_imu].dmpGetQuaternion(&q, fifoBuffer);
                theSensors[which_imu].dmpGetAccel(&aa, fifoBuffer);
                theSensors[which_imu].dmpGetGravity(&gravity, &q);
                theSensors[which_imu].dmpGetLinearAccel(&aaReal, &aa, &gravity);
                theSensors[which_imu].dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
                
                //int16_t acc_i[3];
                //cout <<"Accel range:" << (int)theSensors[which_imu].getFullScaleAccelRange()<<endl;
                //theSensors[which_imu].getAcceleration(acc_i,acc_i+1,acc_i+2);
                float acc[3];
                float shift[3];
                acc[0] = aaWorld.x/8192.0;acc[1] = aaWorld.y/8192.0;acc[2] = aaWorld.z/8192.0;
                //cout <<"g:"<< gravity.x<<" "<< gravity.y<<" "<< gravity.z<<endl;
                for(int i = 0; i < 3; i++){
                    shift[i] = this->the_state.imu_states[which_imu].velocity[i] * dt + 0.5*acc[i]*dt*dt;
                    this->the_state.imu_states[which_imu].position[i] += shift[i];
                    this->the_state.imu_states[which_imu].velocity[i] += acc[i]*dt;
                }
                //cout <<"acc:"<<acc[0]<<" "<<acc[1]<<" "<<acc[2]<<endl;
                this->the_state.imu_states[which_imu].time_stamp = now_stamp;
                pthread_rwlock_unlock(&glove_state_rwlock);
                fwrite(acc,sizeof(float),3,pf_output);
                // fwrite(this->the_state.imu_states[which_imu].velocity,sizeof(float),3,pf_output);
                // fwrite(this->the_state.imu_states[which_imu].position,sizeof(float),3,pf_output);
            }
            
        #endif

        #ifdef OUTPUT_READABLE_YAWPITCHROLL
            // display Euler angles in degrees
            theSensors[which_imu].dmpGetQuaternion(&q, fifoBuffer);
            theSensors[which_imu].dmpGetGravity(&gravity, &q);
            theSensors[which_imu].dmpGetYawPitchRoll(ypr, &q, &gravity);
            if(which_itr % 5 == 0)
            printf("ypr %5.5f     %5.5f     %5.5f\n",ypr[0] * 180/M_PI ,ypr[1] * 180/M_PI, ypr[2] * 180/M_PI);
        #endif

        #ifdef OUTPUT_READABLE_WORLDACCEL
            // display initial world-frame acceleration, adjusted to remove gravity
            // and rotated based on known orientation from quaternion
            theSensors[which_imu].dmpGetQuaternion(&q, fifoBuffer);
            theSensors[which_imu].dmpGetAccel(&aa, fifoBuffer);
            theSensors[which_imu].dmpGetGravity(&gravity, &q);
            theSensors[which_imu].dmpGetLinearAccel(&aaReal, &aa, &gravity);
            theSensors[which_imu].dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
            if(which_itr % 5 == 0)
            printf("aworld %5.5d     %5.5d     %5.5d\n",aaWorld.x ,aaWorld.y, aaWorld.z);
        #endif

        #ifdef OUTPUT_READABLE_REALACCEL
            // display real acceleration, adjusted to remove gravity
            theSensors[which_imu].dmpGetQuaternion(&q, fifoBuffer);
            theSensors[which_imu].dmpGetAccel(&aa, fifoBuffer);
            theSensors[which_imu].dmpGetGravity(&gravity, &q);
            theSensors[which_imu].dmpGetLinearAccel(&aaReal, &aa, &gravity);
            if(which_itr % 5 == 0)
            printf("areal %5.5d     %5.5d     %5.5d\n",aaReal.x ,aaReal.y, aaReal.z);
        #endif

        #ifdef OUTPUT_READABLE_EULER
            // display Euler angles in degrees
            theSensors[which_imu].dmpGetQuaternion(&q, fifoBuffer);
            theSensors[which_imu].dmpGetEuler(euler, &q);
            if(which_itr % 5 == 0)
            printf("euler %5.5f     %5.5f     %5.5f\n",euler[0] * 180/M_PI ,euler[1] * 180/M_PI, euler[2] * 180/M_PI);
        #endif
    }
    return 0;
}

pthread_rwlock_t* glove::get_rwlock()
{
    return &glove_state_rwlock;
}

glove_state* glove::get_glove_state_ptr()
{
    return &the_state;
}