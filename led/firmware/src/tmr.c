#include "tmr.h"
#include "app.h"
volatile uint32_t msCount=0;
void WDTtimer2Init(){
    T2CONbits.ON=0;
    T2CONbits.TCS=0;
    T2CONbits.TCKPS=0x7;
    T2CONbits.T32=0;
    TMR2=0;
    PR2=3125; //312500*256=80000000;  //1S is divided by 100,  activity the time interrupt space is 10ms;
    IPC2bits.T2IP=0x1;
    IPC2bits.T2IS=0x1;
    IEC0bits.T2IE=1;
    T2CONbits.ON=1;
}
void delayN10ms(uint8_t msTime)
{
    uint32_t msCountPresent=msCount;
    while(msCount<msCountPresent+msTime);
    

}