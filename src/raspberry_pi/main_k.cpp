#include<iostream>
#include<string>
#include <unistd.h>
#include <bcm2835.h>
#define _USE_MATH_DEFINES
#include<math.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include"dragon.h"
//#include"MPU6050_6Axis_MotionApps20.hpp"
#define MPU6050_INCLUDE_DMP_MOTIONAPPS20
#include"MPU6050.h"
#include"glove.h"
#include"I2Cdev.h"
#include"helper_3dmath.h"
#include<args.hxx>
#include<vector>



using namespace std;

int imu_i2c_addr = 0x68;
int multi_i2c_addr = 0x70;
bool tcaselect(uint8_t i);

int main(int argc, char** argv)
{
    /////////////////////////////////////////////////
    /////step 1 parse args                       ////
    ////////////////////////////////////////////////
    args::ArgumentParser parser("This program is used to get hand motion", "This goes after the options.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});

    args::Group config_group(parser, "This group is all exclusive:", args::Group::Validators::DontCare);
    args::ValueFlag<int> a_imu_num(config_group, "int", "imu num", {"imun"});
    args::ValueFlag<string> a_calib_save_path(config_group, "string", "Calibration data save path", {"calibpa"});
    args::ValueFlag<int> a_net_port(config_group,"int","net port to communicate",{"port"});
    
    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Completion& e)
    {
        std::cout << e.what();
        return 0;
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError& e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    /////////////////////////////////////////////////
    /////step 2 calibration                     ////
    ////////////////////////////////////////////////

    int imu_num = args::get(a_imu_num);

    string calib_save_path = args::get(a_calib_save_path);
    if(mkdir(calib_save_path.c_str(),0777) == -1){
        cout <<"Cannot create folder :"<<calib_save_path<<endl;
        cout <<"because:";
        if(errno == EEXIST){
            cout <<"The folder already exists!"<<endl;
        }else{
            cout <<"Unkown error, error code:"<<(int)errno<<endl;
            return -1;
        }
    }; 
    string calib_file_name = "calib_ag_data";

    I2Cdev::initialize();

    vector<MPU6050> theSensors;
    vector<uint16_t> packetSizes;    // expected DMP packet size (default is 42 bytes)
    uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU

    for(int which_imu = 0; which_imu < imu_num; which_imu++){
        //switch to the i2c line 
        if(!tcaselect(which_imu)){
            cout <<"Cannot switch to i2c line:"<<which_imu<<endl;
            continue;
        }else{
            cout <<"Switch i2c bus to id:"<<which_imu<<endl;
        }

        // MPU control/status vars
        uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)        
        
        MPU6050 theSensor;

        cout <<"Testing device connections..."<<endl;
        if(theSensor.testConnection()){
            cout <<"MPU6050 connection successful"<<endl;
        }else{
            cout <<"MPU6050 connection failed"<<endl;
            cout <<"device id:" << (int)theSensor.getDeviceID()<<endl;
            return -1;
        }
        cout << "Initializing I2C devices..."<<endl;
        theSensor.initialize();
        

        FILE* pf_calib_file;
        bool recalib_needed = false;
        pf_calib_file = fopen(string(calib_save_path+calib_file_name+"_"+std::to_string(which_imu)+".bin").c_str(),"rb");
        if(pf_calib_file == NULL){
            cout <<"The calibration file of imu:"<<which_imu<<" doesn't exist. recalibration need!"<<endl;
            pf_calib_file = fopen(string(calib_save_path+calib_file_name+"_"+std::to_string(which_imu)+".bin").c_str(),"wb");
            cout <<"Please let the imu stand firmly up. imu id:"<<which_imu<<endl;
            getchar();
            recalib_needed = true;
        }
        // theSensor.setFullScaleAccelRange(0);
        sleep(1);

        cout <<"Initializing DMP..."<<endl;
        devStatus = theSensor.dmpInitialize();

        if (devStatus == 0) {
            // Calibration Time: generate offsets and calibrate our MPU6050
            theSensor.CalibrateAccel(recalib_needed,pf_calib_file,6);
            theSensor.CalibrateGyro(recalib_needed,pf_calib_file,6);
            fclose(pf_calib_file);
            theSensor.PrintActiveOffsets();
            usleep(2000);
            // turn on the DMP, now that it's ready
            cout <<"Enabling DMP..."<<endl;
            theSensor.setDMPEnabled(true);

            // enable Arduino interrupt detection
            cout <<"Enabling interrupt detection (Arduino external interrupt ";
            cout <<")..."<<endl;
            // theSensor.setInterruptMode(true);
            // theSensor.setInterruptLatchClear(false);
            mpuIntStatus = theSensor.getIntStatus();
            cout <<"MPU_INT_STATUS"<<(int)mpuIntStatus<<endl;

            // set our DMP Ready flag so the main loop() function knows it's okay to use it
            cout <<"DMP ready! Waiting for first interrupt..."<<endl;
            
            // get expected DMP packet size for later comparison
            uint16_t packetSize = theSensor.dmpGetFIFOPacketSize();
            packetSizes.emplace_back(packetSize);
        } else {
            // ERROR!
            // 1 = initial memory load failed
            // 2 = DMP configuration updates failed
            // (if it's going to break, usually the code will be 1)
            cout << "DMP Initialization failed (code ";
            cout << (int)devStatus;
            cout <<")"<<endl;
            cout << "1 = initial memory load failed" << endl;
            cout << "2 = DMP configuration updates failed" <<endl;
            cout << "(if it's going to break, usually the code will be 1)" <<endl;
            return -1;
        }
        theSensors.emplace_back(theSensor);
        cout <<"mpu id:"<<which_imu<<" init done."<<endl;
    }
    
    //////////////////////////////////////////////////////////////
    /////// setting dragon and glove master                   ////
    /////////////////////////////////////////////////////////////
    glove theGlove(imu_num,theSensors,packetSizes);
    Dragon theDragon(args::get(a_net_port),theGlove.get_rwlock(),theGlove.get_glove_state_ptr());
    theDragon.init_net();
    theDragon.Start();
    //////////////////////////////////////////////////////////////
    ////// loop to get imu state                             ////
    //////////////////////////////////////////////////////////////
    // orientation/motion vars

    FILE* pf_save;
    pf_save = fopen("motion.bin","wb");

    for(int which_itr = 0; which_itr < 550000; which_itr++){
        if(which_itr % 1000 == 0){
            cout <<which_itr<<endl;
        }
        for(int which_imu = 0; which_imu < imu_num; which_imu++){
            //switch to the i2c line 
            if(!tcaselect(which_imu)){
                cout <<"Cannot switch to i2c line:"<<which_imu<<endl;
                continue;
            }
            theGlove.update_state(which_imu,pf_save);
        }
    }

    theDragon.landing();
    cout <<"before join"<<endl;
    theDragon.Join();
    cout <<"join end"<<endl;

    fclose(pf_save);

    return 0;
}

bool tcaselect(uint8_t i) {
    if (i > 7) return false;
    return I2Cdev::writeBytes(multi_i2c_addr, 1 << i, 0,NULL);
}