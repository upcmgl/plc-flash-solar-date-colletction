#include "baseFun.h"

void bufferClear(uint8_t *buffer,uint16_t lens)
{
    memset(buffer,0,lens);
}
uint8_t num2BCD(uint8_t num)
{
    uint8_t temp=0xFF;
    temp=(num/10)&temp<<4;
    temp=(temp|0xFF)&(num%10);
    return temp;
}