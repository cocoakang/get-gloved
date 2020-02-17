#include <hidef.h>
#include "derivative.h"
#include "adc.h"
#include "buzzer.h"
#include "SCItx.h"
#pragma DATA_SEG MY_ZEROPAGE
unsigned char near Sample_X;
unsigned char near Sample_Y;
unsigned char near Sample_Z;
unsigned char near Sensor_Data[8];
unsigned char near countx,county ;
signed int near accelerationx[2], accelerationy[2];
signed long near velocityx[2], velocityy[2];
signed long near positionX[2];
signed long near positionY[2];
signed long near positionZ[2];
unsigned char near direction;
unsigned long near sstatex,sstatey;
#pragma DATA_SEG DEFAULT
void init(void);
void Calibrate(void);
void data_transfer(void);
void concatenate_data(void);
void movement_end_check(void);
void position(void);
void main (void)
{
 init();
 get_threshold();
 do
 {
 position();
 }while(1);
}
/*******************************************************************************
 The purpose of the calibration routine is to obtain the value of the reference threshold.
 It consists on a 1024 samples average in no-movement condition.
********************************************************************************/
void Calibrate(void)
{
    unsigned int count1;
    count1 = 0;
    do{
        ADC_GetAllAxis();
        sstatex = sstatex + Sample_X; // Accumulate Samples
        sstatey = sstatey + Sample_Y;
        count1++;
    }while(count1!=0x0400); // 1024 times
    sstatex=sstatex>>10; // division between 1024
    sstatey=sstatey>>10;
}
/*****************************************************************************************/
/******************************************************************************************
This function obtains magnitude and direction
In this particular protocol direction and magnitude are sent in separate variables.
Management can be done in many other different ways.
*****************************************************************************************/
void data_transfer(void)
{
 signed long positionXbkp;
 signed long positionYbkp;
 unsigned int delay;
 unsigned char posx_seg[4], posy_seg[4];
 if (positionX[1]>=0) { //This line compares the sign of the X direction data
 direction= (direction | 0x10); //if its positive the most significant byte
 posx_seg[0]= positionX[1] & 0x000000FF; // is set to 1 else it is set to 8
 posx_seg[1]= (positionX[1]>>8) & 0x000000FF; // the data is also managed in the
 // subsequent lines in order to
 posx_seg[2]= (positionX[1]>>16) & 0x000000FF; // be sent. The 32 bit variable must be
 posx_seg[3]= (positionX[1]>>24) & 0x000000FF; // split into 4 different 8 bit
 // variables in order to be sent via
 // the 8 bit SCI frame

  }


 else {direction=(direction | 0x80);
 positionXbkp=positionX[1]-1;
 positionXbkp=positionXbkp^0xFFFFFFFF;
 posx_seg[0]= positionXbkp & 0x000000FF;
 posx_seg[1]= (positionXbkp>>8) & 0x000000FF;
 posx_seg[2]= (positionXbkp>>16) & 0x000000FF;
 posx_seg[3]= (positionXbkp>>24) & 0x000000FF;
 }


 if (positionY[1]>=0) { // Same management than in the previous case
 direction= (direction | 0x08); // but with the Y data.
 posy_seg[0]= positionY[1] & 0x000000FF;
 posy_seg[1]= (positionY[1]>>8) & 0x000000FF;
 posy_seg[2]= (positionY[1]>>16) & 0x000000FF;
 posy_seg[3]= (positionY[1]>>24) & 0x000000FF;
 }

 else {direction= (direction | 0x01);
 positionYbkp=positionY[1]-1;
 positionYbkp=positionYbkp^0xFFFFFFFF;
 posy_seg[0]= positionYbkp & 0x000000FF;
 posy_seg[1]= (positionYbkp>>8) & 0x000000FF;
 posy_seg[2]= (positionYbkp>>16) & 0x000000FF;
 posy_seg[3]= (positionYbkp>>24) & 0x000000FF;
 }

 delay = 0x0100;

 Sensor_Data[0] = 0x03;
 Sensor_Data[1] = direction;
 Sensor_Data[2] = posx_seg[3];
 Sensor_Data[3] = posy_seg[3];
 Sensor_Data[4] = 0x01;
 Sensor_Data[5] = 0x01;
 Sensor_Data[6] = END_OF_FRAME;

 while (--delay);

 SCITxMsg(Sensor_Data); // Data transferring function
 while (SCIC2 & 0x08);
}
/*****************************************************************************************/
/******************************************************************************************
This function returns data format to its original state. When obtaining the magnitude and
direction of the position, an inverse two's complement is made. This function makes the two's
complement in order to return the data to it original state.
It is important to notice that the sensibility adjustment is greatly impacted here, the amount
of "ones" inserted in the mask must be equivalent to the "ones" lost in the shifting made in
the previous function upon the sensibility modification.
 ******************************************************************************************/
void data_reintegration(void)
{
 if (direction >=10) 
  {positionX[1]= positionX[1]|0xFFFFC000;} // 18 "ones" inserted. Same size as the
 //amount of shifts

 direction = direction & 0x01;
 if (direction ==1)

 {positionY[1]= positionY[1]|0xFFFFC000;}
}
/******************************************************************************************
This function allows movement end detection. If a certain number of acceleration samples are
equal to zero we can assume movement has stopped. Accumulated Error generated in the velocity
calculations is eliminated by resetting the velocity variables. This stops position increment
and greatly eliminates position error.
******************************************************************************************/
void movement_end_check(void)
{
 if (accelerationx[1]==0) //we count the number of acceleration samples that equals cero
 { countx++;}
 else { countx =0;}

 if (countx>=25) //if this number exceeds 25, we can assume that velocity is cero
 {
 velocityx[1]=0;
 velocityx[0]=0;
 }

 if (accelerationy[1]==0) //we do the same for the Y axis
 { county++;}
 else { county =0;}

 if (county>=25)
 {
 velocityy[1]=0;
 velocityy[0]=0;
 }
}
/*****************************************************************************************/
/******************************************************************************************
This function transforms acceleration to a proportional position by integrating the
acceleration data twice. It also adjusts sensibility by multiplying the "positionX" and
"positionY" variables.
This integration algorithm carries error, which is compensated in the "movenemt_end_check"
subroutine. Faster sampling frequency implies less error but requires more memory. Keep in
mind that the same process is applied to the X and Y axis.
*****************************************************************************************/
void position(void)
{
    unsigned char count2 ;
    count2=0;

    do{

        ADC_GetAllAxis();
        accelerationx[1]=accelerationx[1] + Sample_X; //filtering routine for noise attenuation
        accelerationy[1]=accelerationy[1] + Sample_Y; //64 samples are averaged. The resulting 
        //average represents the acceleration of
        //an instant
        count2++;

    }while (count2!=0x40); // 64 sums  of the acceleration sample


    accelerationx[1]= accelerationx[1]>>6; // division by 64
    accelerationy[1]= accelerationy[1]>>6;

    accelerationx[1] = accelerationx[1] - (int)sstatex; //eliminating zero reference
    //offset of the acceleration data
    accelerationy[1] = accelerationy[1] - (int)sstatey; // to obtain positive and negative
    //acceleration


    if ((accelerationx[1] <=3)&&(accelerationx[1] >= -3)) //Discrimination window applied
    {accelerationx[1] = 0;} // to the X axis acceleration
    //variable

    if ((accelerationy[1] <=3)&&(accelerationy[1] >= -3))
    {accelerationy[1] = 0;}

    //first X integration:
    velocityx[1]= velocityx[0]+ accelerationx[0]+ ((accelerationx[1] -accelerationx[0])>>1);
    //second X integration:
    positionX[1]= positionX[0] + velocityx[0] + ((velocityx[1] - velocityx[0])>>1);
    //first Y integration:
    velocityy[1] = velocityy[0] + accelerationy[0] + ((accelerationy[1] -accelerationy[0])>>1);
    //second Y integration:
    positionY[1] = positionY[0] + velocityy[0] + ((velocityy[1] - velocityy[0])>>1);

    accelerationx[0] = accelerationx[1]; //The current acceleration value must be sent
    //to the previous acceleration
    accelerationy[0] = accelerationy[1]; //variable in order to introduce the new
    //acceleration value.

    velocityx[0] = velocityx[1]; //Same done for the velocity variable
    velocityy[0] = velocityy[1];


    positionX[1] = positionX[1]<<18; //The idea behind this shifting (multiplication)
    //is a sensibility adjustment.
    positionY[1] = positionY[1]<<18; //Some applications require adjustments to a
    //particular situation
    //i.e. mouse application
    data_transfer();

    positionX[1] = positionX[1]>>18; //once the variables are sent them must return to
    positionY[1] = positionY[1]>>18; //their original state
    movement_end_check();

    positionX[0] = positionX[1]; //actual position data must be sent to the
    positionY[0] = positionY[1]; //previous position

    direction = 0; // data variable to direction variable reset
}
/*****************************************************************************************/