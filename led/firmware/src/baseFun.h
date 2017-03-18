#ifndef _BASE_FUN_H    /* Guard against multiple inclusion */
#define _BASE_FUN_H

#include "app.h"

 
//
//#define WEEKDAY     9       //星期
//#define CENTURY     8       //年低字节
//#define MSECOND_H   7       //毫秒高字节，目前仅提供10毫秒精度
//#define MSECOND_L   6       //毫秒低字节，目前仅提供10毫秒精度
//#define YEAR		5       //年，不带世纪
//#define MONTH		4       //月
//#define DAY  		3       //日
//#define HOUR		2       //时
//#define MINUTE		1       //分
//#define SECOND		0       //秒


// datetime
#define WEEKALARM   10
#define HOURALARM   9
#define MINALARM    8
#define YEAR		6       //年，不带世纪
#define MONTH		5       //月
#define DAY  		4
#define WEEKDAY 	3       //日
#define HOUR		2       //时
#define MINUTE		1       //分
#define SECOND		0       //秒

void bufferClear(uint8_t *buffer,uint16_t lens);

uint8_t num2BCD(uint8_t num);
void    mem_cpy(void *dst,void *src,uint16_t len);
bool is_valid_bcd(uint8_t *bcd,uint8_t bytes);
uint8_t  compare_string(uint8_t *str1,uint8_t *str2,uint16_t len);
uint8_t BCD2byte(uint8_t bcd);
#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
