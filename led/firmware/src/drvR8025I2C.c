/*******************************************************************************
********************************************************************************
**
**  Filename:       drvR8025T.c
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
#include "drvR8025I2C.h"
#include "drv_i2c.h"
#include "../include/stdint.h"
//#define DRV_I2C_ERR_DEBUG

#define CPO_COUNT_US  (SYS_FREQUENCY/2/1000000)
#define  R8025T_READ_ADDRESS 0x65
#define  R8025T_WRITE_ADDRESS 0x64
uint8_t drv_R8025T_read(uint8_t addr,uint8_t* data,uint8_t len);  
void drv_I2C_init(void)
{

    //TODO:获取信号量

    DRV_I2C_Initialize();

    //TODO:释放信号量
}
//
uint8_t drv_R8025T_check_read(uint8_t* buffer)
{
    uint8_t data[32];
    volatile uint8_t try_count=0;
     while(1)
    {
        try_count++;
        if(try_count > 10)
        {
            break;
        }         
//        if(try_count>1)
//        {
//           tpos_sleep(100);
//        }    
        if(drv_R8025T_read(0,data,32)==32)
        {
            if(compare_string(data,data+16,16)!=0)//读到不同，需要重新读取
            {
                continue;
            }
            if((is_valid_bcd(data,3)==false) || (is_valid_bcd(data+4,3)==false))
            {
                continue;
            }
            if(data[0]>=0x60) continue;
            if(data[1]>=0x60) continue;
            if(data[2]>=0x24) continue;
            if(data[4]>=0x32) continue;
            if(data[5]>=0x13) continue;
        //    if(data[4]==0x00) continue;
         //   if(data[5]==0x00) continue;
            mem_cpy(buffer,data,16);
            return 16;
        }

    }
    return 0;
}
bool drv_R8025T_init(void)
{
    uint8_t data[32];
    uint8_t try_count=0;
    uint8_t update_flag;
    while(1)
    {
        if(drv_R8025T_read(0,data,32)==32)
        {
            if(compare_string(data,data+16,16)==0)
            {
                break;
            }
        }
        try_count++;
        if(try_count > 10)
        {
            break;
        }
    }
    if(try_count <= 10)
    {
        if(data[0x0E]&0x02)
        {
            //initial 
            data[0] = 0x00;//秒
            data[1] = 0x00;//分
            data[2] = 0x00;//时
            data[3] = 0x10;//星期
            data[4] = 0x01;//日
            data[5] = 0x01;//月
            data[6] = 0x15;//年
            data[7] = 0x00;
            drv_R8025T_write(0x00,data,8);

            data[0x0D] = 0x00;
            data[0x0E] = 0x00;
            data[0x0F] = 0x60;
            drv_R8025T_write(0x0D,&data[0x0D],3);
        }
        else
        {
            update_flag = false;
            if(data[0]>=0x60) update_flag = true;
            if(data[1]>=0x60) update_flag = true;
            if(data[2]>=0x24) update_flag = true;
            if(data[4]>=0x32) update_flag = true;
            if(data[5]>=0x13) update_flag = true;
            if(data[4]==0x00) update_flag = true;
            if(data[5]==0x00) update_flag = true;
            if(update_flag == true)
            {
                data[0] = 0x00; //秒
                data[1] = 0x00; //分
                data[2] = 0x00; //时
                data[3] = 0x10; //星期
                data[4] = 0x01; //日
                data[5] = 0x01; //月
                data[6] = 0x15; //年
                data[7] = 0x00;
                drv_R8025T_write(0x00, data, 8);
            }
            update_flag = false;
            //检查寄存器的值是否正确
            if(data[0x0F] != 0x60)
            {
                data[0x0F] = 0x60;//补偿间隔2s  UIE TIE AIE RESET为0
                update_flag = true;
            }

            if(data[0x0E] != 0x00)
            {

                data[0x0E] = 0x00;  //启动温度补偿  UF TF AF为0
                update_flag = true;
            }

            if(data[0x0D] != 0x00)
            {
                data[0x0D] = 0x00;
                update_flag = true;
            }
            if(update_flag == true)
            {
                drv_R8025T_write(0x0D,&data[0x0D],3);
            }
        }
    }
}
//note  操作必须在0.95s之内完成
/**
 * 
 * @param addr rtc's register initial address.
 * @param data the date array we will read.
 * @param len  the array's length.
 * @return 
 */
uint8_t drv_R8025T_read(uint8_t addr,uint8_t* data,uint8_t len)
{
    uint8_t resault=0;
    uint8_t idx;
    uint8_t try_count;
    uint32_t start_tick;

    //TODO:获取信号量，如果用了tpos_enterCriticalSection()就不用信号量了

//    tpos_enterCriticalSection();
    try_count = 0;
TRY_AGAIN:
    resault=0;
    drv_I2C_init();
    if(try_count++>100)
    {
        #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
        __asm__ volatile (" sdbbp 2");
        #endif
        goto R8025T_READ_END;
    }

    DRV_I2C_Errata32MZ_I2CModuleToggle();
    start_tick = _CP0_GET_COUNT();
    while(!DRV_I2C_MasterBusIdle())
    {
        if((_CP0_GET_COUNT() - start_tick)>=(3000*CPO_COUNT_US))
        {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
            #endif
            goto TRY_AGAIN;
        }
    }
    //发送 start
    if(DRV_I2C_MasterStart()==false)
    {
        #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WaitForStartComplete(3000)==false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    //写设备地址
    if(DRV_I2C_ByteWrite(R8025T_WRITE_ADDRESS)==false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WaitForByteWriteToComplete(5000) == false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WriteByteAcknowledged() == false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    //写寄存器地址
    if(DRV_I2C_ByteWrite(addr) == false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WaitForByteWriteToComplete(3000) == false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WriteByteAcknowledged() == false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }

    DRV_I2C_MasterStop();
    if(DRV_I2C_WaitForStopComplete(3000) == false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    

    DRV_I2C_Errata32MZ_I2CModuleToggle();
    start_tick = _CP0_GET_COUNT();
    while(!DRV_I2C_MasterBusIdle())
    {
        if((_CP0_GET_COUNT() - start_tick)>=(3000*CPO_COUNT_US))
        {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
            #endif
            goto TRY_AGAIN;
        }
    }
    //restart
    if(DRV_I2C_MasterStart()==false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WaitForStartComplete(3000)==false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    //写设备地址
    if(DRV_I2C_ByteWrite(R8025T_READ_ADDRESS)==false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WaitForByteWriteToComplete(3000) == false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WriteByteAcknowledged() == false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    for(idx=0;idx<len;idx++)
    {
        if(DRV_I2C_SetUpByteRead()==false)
        {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
            #endif
            goto TRY_AGAIN;
        }
        //while(!DRV_I2C_MasterBusIdle());
        if(DRV_I2C_WaitForReadByteAvailable(3000) == false)
        {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
            #endif
            goto TRY_AGAIN;
        }
        data[idx] = DRV_I2C_ByteRead();   //Read from I2CxRCV
        if(idx<(len-1))
        {
            DRV_I2C_MasterACKSend();               //Send ACK to Slave
        }
        else
        {
            DRV_I2C_MasterNACKSend();               //Send ACK to Slave
        }
        if(DRV_I2C_WaitForACKOrNACKComplete(3000) == false)
        {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
            #endif
            goto TRY_AGAIN;
        }
    }
    resault = idx;

R8025T_READ_END:
    DRV_I2C_MasterStop();
    DRV_I2C_WaitForStopComplete(3000);

//    tpos_leaveCriticalSection();
    //TODO:释放信号量

    return resault;
}
//note  操作必须在0.95s之内完成
/**
 * 
 * @param addr the rtc's register address
 * @param data the date array's address we will write from.
 * @param len  the length we date we will write in.
 * @return 
 */
bool drv_R8025T_write(uint8_t addr,uint8_t* data,uint8_t len)
{
    bool resault=false;
    uint8_t idx;
    uint8_t try_count;
    uint32_t start_tick;

    //TODO:获取信号量，如果用了tpos_enterCriticalSection()就不用信号量了

//    tpos_enterCriticalSection();
    try_count = 0;
TRY_AGAIN:
    resault = false;
    drv_I2C_init();
    if(try_count++>100)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto R8025T_WRITE_END;
    }
    DRV_I2C_Errata32MZ_I2CModuleToggle();
    start_tick = _CP0_GET_COUNT();
    while(!DRV_I2C_MasterBusIdle())
    {
        if((_CP0_GET_COUNT() - start_tick)>=(3000*CPO_COUNT_US))
        {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
            #endif
            goto TRY_AGAIN;
        }
    }
    //发送 start
    if(DRV_I2C_MasterStart()==false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WaitForStartComplete(3000)==false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    //写设备地址
    if(DRV_I2C_ByteWrite(R8025T_WRITE_ADDRESS)==false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WaitForByteWriteToComplete(3000)==false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WriteByteAcknowledged() == false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    //写寄存器地址
    if(DRV_I2C_ByteWrite(addr) == false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WaitForByteWriteToComplete(3000) == false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    if(DRV_I2C_WriteByteAcknowledged() == false)
    {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
        #endif
        goto TRY_AGAIN;
    }
    //写入数据
    for(idx=0;idx<len;idx++)
    {
        if(DRV_I2C_ByteWrite(data[idx]) == false)
        {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
            #endif
            goto TRY_AGAIN;
        }
        if(DRV_I2C_WaitForByteWriteToComplete(3000) == false)
        {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
            #endif
            goto TRY_AGAIN;
        }
        if(DRV_I2C_WriteByteAcknowledged() == false)
        {
            #if defined __DEBUG && defined DRV_I2C_ERR_DEBUG
    __asm__ volatile (" sdbbp 2");
            #endif
            goto TRY_AGAIN;
        }     
    }

 R8025T_WRITE_END:
    DRV_I2C_MasterStop();
    DRV_I2C_WaitForStopComplete(3000);

//    tpos_leaveCriticalSection();
    //TODO:释放信号量

    return resault;
}