/*******************************************************************************
********************************************************************************
**
**  Filename:       OStime.c
**  Copyright(c):   2017 Topscomm. All right reserved.
**  Author:         mgl
**  Date:           2017.3.25
**  Device:         MicroInverter Collector, MCU: PIC32MX664F128L
**  Modify by:
**  Modify date:
**  Version:        1.0.0
**  Describe:
**
**
********************************************************************************
*******************************************************************************/
#include "osTimer.h"

//system clock
 uint8_t datetime[10];
 uint8_t initialTime[]={0,59,3,0,1,1,17};
 uint8_t timer0=1;
 uint8_t timer1=0;
 uint8_t extension =0x12;
 uint8_t ctr   =0x50;
 
 
 struct sysTimeStr sysTime;
 /**
  * set second interrupt;
  */
 void secInterrupt(uint8_t secs)
 {
     uint8_t rtc[16];
     rtc[0x0D] =0;
     drv_R8025T_write(0x0D,&rtc[0x0D],1);   //TE==0;
     rtc[0x0B]=secs;               //interrupt every secs s.
     rtc[0x0C]=0;
     rtc[0x0D]=0x12;        
     drv_R8025T_write(0x0B,&rtc[0x0B],3);
     rtc[0x0E]=0x0;
     drv_R8025T_write(0x0E,&rtc[0x0E],1);
     rtc[0x0F]=0x50;
     drv_R8025T_write(0x0F,&rtc[0x0F],1); 
 }
 /**
  * 
  * @param mins
  */
 void minInterrupt(uint8_t mins)
 {  
     uint8_t rtc[16];
     rtc[0x0D] =0;
     drv_R8025T_write(0x0D,&rtc[0x0D],1);   //TE==0;
     rtc[0x0B]=mins;               //interrupt every mins s.
     rtc[0x0C]=0;
     rtc[0x0D]=0x13;            // source clock is minute.
     drv_R8025T_write(0x0B,&rtc[0x0B],3);
     rtc[0x0E]=0x0;
     drv_R8025T_write(0x0E,&rtc[0x0E],1);
     rtc[0x0F]=0x50;
     drv_R8025T_write(0x0F,&rtc[0x0F],1); 
 }
 
 /**
  * 
  * @param clock
  */
 void setAlarmClockStart(uint8_t clock)
 {
     uint8_t rtc[16];
     rtc[0xD] = 0x40;    //WADA ==1;
     drv_R8025T_write(0x0D,&rtc[0x0D],1); 
     rtc[0x8] = 0x00;
     rtc[0x9] = clock;
     rtc[0xA] = 0x80;
     drv_R8025T_write(0x08,&rtc[0x08],3); 
     rtc[0xE] = 0x0;
     rtc[0xF] = 0x48;
     drv_R8025T_write(0x0E,&rtc[0x0E],2); 
     
 }

bool __attribute__((optimize ("O0"))) system_get_rtc_time(void)
{
    uint8_t rtc[16];
    uint8_t write_rtc[16];
    uint8_t data_len;
    uint8_t idx;
    data_len=drv_R8025T_check_read(rtc);
    if(data_len == 16)
    {
       if(rtc[0x0E]&0x02)
        {
//            gSystemInfo.clock_flag=0x55;
            rtc[0x0D] = 0x00;
            rtc[0x0E] = 0x00;
            rtc[0x0F] = 0x60;
            drv_R8025T_write(0x0D,&rtc[0x0D],3);//the normal runtime found vlf==1,then clear it .
            
            DateTime2RTCBCD(datetime,rtc);
            //wtrite 8025T week is according to bits.
            mem_cpy(write_rtc,rtc,16);
            write_rtc[3] = BCD2byte(write_rtc[3]);
            if(write_rtc[3]==7)
            {
                write_rtc[3] = 0;
            }
            write_rtc[3] = 1<<write_rtc[3];
            drv_R8025T_write(0,write_rtc,7);//when VLF==1,then write the present time into RTC.      
        }
        //rtc[3]is present week.
        for(idx=0;idx<7;idx++)
        {
            if(rtc[3]&(0x01<<idx))
            {
                break;
            }
        }
        if(idx==0)
        {
            idx = 7;
        }
        rtc[3] = byte2BCD(idx);
        RTCBCD2DateTime(datetime,rtc);
        return true;
    }
    return false;
}
//INT8U *new_time 格式与datetime相同

void __attribute__((optimize ("O0"))) system_set_rtc_time(uint8_t *new_time,struct nandFlashInfoStr *nandFlashInfo,struct MonthInfoStr *monthInfo)
{
    uint8_t rtc[8];
    uint8_t write_rtc[8];

    DateTime2RTCBCD(new_time,rtc);

    mem_cpy(write_rtc,rtc,8);
    write_rtc[3] = BCD2byte(write_rtc[3]);
    if(write_rtc[3]==7)
    {
        write_rtc[3] = 0;
    }
    write_rtc[3] = 1<<write_rtc[3];
    drv_R8025T_write(0,write_rtc,7);
    
    //when set the system time,we should make the system information block get the system time;
    nandFlashInfo ->yearMonthCtl[0].year = num2BCD(new_time[6]);
    nandFlashInfo ->yearMonthCtl[0].monthWithIndex[0].month = num2BCD(new_time[5]);
    
    monthInfo ->structLens = sizeof(struct MonthInfoStr);
    monthInfo ->month =num2BCD(new_time[5]);
    monthInfo ->dayWithIndex[0].day =num2BCD(new_time[4]);
    
    sysTime.timeDate.min   = num2BCD(new_time[1]);
    sysTime.timeDate.hour  = num2BCD(new_time[2]);
    sysTime.timeDate.day   = num2BCD(new_time[4]);
    sysTime.timeDate.month = num2BCD(new_time[5]);
    sysTime.timeDate.year  = num2BCD(new_time[6]);
    
}