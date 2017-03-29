/*******************************************************************************
********************************************************************************
**
**  Filename:       baseFunction.c
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
#include "baseFun.h"


/**
 * //////set all bits in buffer as 0x00;
 * @param buffer
 * @param lens
 */
void bufferClear(uint8_t *buffer,uint16_t lens)
{
    memset(buffer,0,lens);
}

uint8_t __attribute__((optimize("O0"))) num2BCD(uint8_t num)
{
    uint8_t temp=0xFF;
    temp=((num/10)&temp)<<4;
    temp=temp|(0xFF&(num%10));
    return temp;
}

void    mem_cpy(void *dst,void *src,uint16_t len)
{
   uint16_t i;

   for(i=0;i<len;i++)
   {
       ((uint8_t *)dst)[i] = ((uint8_t *)src)[i];
   }

}

bool is_valid_bcd(uint8_t *bcd,uint8_t bytes)
{
   uint8_t i;
   for(i=0;i<bytes;i++)
   {
      if((bcd[i] & 0x0F) > 0x09) return false;
      if((bcd[i] & 0xF0) > 0x90) return false;
   }
   return true;
}

/**
 * 
 * @param str1  string 1
 * @param str2  string 2
 * @param len   string length
 * @return 
 */
uint8_t  compare_string(uint8_t *str1,uint8_t *str2,uint16_t len)
{
    while(len > 0)
    {
        if(*str1 < *str2) return -1;
        if(*str1 > *str2) return 1;
        str1++;
        str2++;
        len--;
    }
    return 0;
}
uint8_t BCD2byte(uint8_t bcd)
{
    volatile uint8_t a=( bcd & 0x000F);
    volatile uint8_t b=((bcd>>4) & 0x000F) * 10;
    return a+b;
 //  return ((bcd>>4) & 0x000F) * 10 + ( bcd & 0x000F );
}

//
//  trans bytes to BCD
//
uint8_t byte2BCD(uint8_t bVal)
{
    uint8_t ch;

    ch = ((bVal/10)<<4) + (bVal%10);

    return ch;
}
/*+++
  ����: ��BCD��ʽʱ��תΪ�ֽڸ�ʽ
  ����: 
      INT8U *dt Ŀ��ת��ֵ
      INT8U *nValue ��ת��ֵ�������� �ӵ͵��߸�ʽΪ:��-��-ʱ-����-��-��-��
  ����: 
---*/
void RTCBCD2DateTime(uint8_t *dt,uint8_t *nValue)
{
     dt[SECOND] = BCD2byte(nValue[0]);
     dt[MINUTE] = BCD2byte(nValue[1]);
     dt[HOUR] =   BCD2byte(nValue[2]);
     dt[WEEKDAY] =BCD2byte(nValue[3]);	
     dt[DAY] =    BCD2byte(nValue[4]);
     dt[MONTH] =  BCD2byte(nValue[5]);
     dt[YEAR] =   BCD2byte(nValue[6]);
     dt[MINALARM] =   BCD2byte(nValue[8]);
     dt[HOURALARM] =   BCD2byte(nValue[9]);
     dt[WEEKALARM] =   BCD2byte(nValue[10]);
}
/**
 * 
 * @param dt
 * @param nValue
 */
void  DateTime2RTCBCD(uint8_t *dt,uint8_t *nValue)
{
     nValue[0]=byte2BCD(dt[SECOND]);
     nValue[1]=byte2BCD(dt[MINUTE]);
     nValue[2]=byte2BCD(dt[HOUR]);
     nValue[3]=byte2BCD(dt[WEEKDAY]);	
     nValue[4]=byte2BCD(dt[DAY]);
     nValue[5]=byte2BCD(dt[MONTH]);
     nValue[6]=byte2BCD(dt[YEAR]);
}
/**
 * 
 * @param dt
 * @param nValue
 */
void  DateTime2645BCD(uint8_t *dt,uint8_t *nValue)
{
     nValue[0]=byte2BCD(dt[SECOND]);
     nValue[1]=byte2BCD(dt[MINUTE]);
     nValue[2]=byte2BCD(dt[HOUR]);
     nValue[3]=byte2BCD(dt[DAY]);
     nValue[4]=byte2BCD(dt[MONTH]);
     nValue[5]=byte2BCD(dt[YEAR]);
}