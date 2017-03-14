#include "osTimer.h"
//system clock
volatile uint8_t datetime[10];
bool __attribute__((optimize ("O0"))) system_get_rtc_time(void)
{
    uint8_t rtc[16];
    uint8_t write_rtc[16];
    uint8_t data_len;
    uint8_t idx;

    #ifndef __SOFT_SIMULATOR__
    data_len=drv_R8025T_check_read(rtc);
    if(data_len == 16)
    {
        if(rtc[0x0E]&0x02)
        {
//            gSystemInfo.clock_flag=0x55;
            rtc[0x0D] = 0x00;
            rtc[0x0E] = 0x00;
            rtc[0x0F] = 0x60;
            drv_R8025T_write(0x0D,&rtc[0x0D],3);//�������ж�ȡʱ����VLFλ��1������Ҫ���������
            
            DateTime2RTCBCD(datetime,rtc);
            //д��8025T�������ǰ�λ��ʶ
            mem_cpy(write_rtc,rtc,16);
            write_rtc[3] = BCD2byte(write_rtc[3]);
            if(write_rtc[3]==7)
            {
                write_rtc[3] = 0;
            }
            write_rtc[3] = 1<<write_rtc[3];
            drv_R8025T_write(0,write_rtc,7);//��VLF��1�󣬽�����ʱ��д��ʱ��оƬ       
        }
        //rtc[3]�����ڣ�8025T�ǰ�λ��ʶ��
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
    #endif

    return false;
}
//INT8U *new_time ��ʽ��datetime��ͬ
void __attribute__((optimize ("O0"))) system_set_rtc_time(uint8_t *new_time)
{
    uint8_t rtc[8];
    uint8_t write_rtc[8];

    DateTime2RTCBCD(new_time,rtc);

    #ifndef __SOFT_SIMULATOR__  
    //д��8025T�������ǰ�λ��ʶ
    mem_cpy(write_rtc,rtc,8);
    write_rtc[3] = BCD2byte(write_rtc[3]);
    if(write_rtc[3]==7)
    {
        write_rtc[3] = 0;
    }
    write_rtc[3] = 1<<write_rtc[3];
    drv_R8025T_write(0,write_rtc,7);
    #endif
}