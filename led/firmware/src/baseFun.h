#ifndef _BASE_FUN_H    /* Guard against multiple inclusion */
#define _BASE_FUN_H

#include "app.h"

// datetime 

#define WEEKDAY     9       //����
#define CENTURY     8       //����ֽ�
#define MSECOND_H   7       //������ֽڣ�Ŀǰ���ṩ10���뾫��
#define MSECOND_L   6       //������ֽڣ�Ŀǰ���ṩ10���뾫��
#define YEAR		5       //�꣬��������
#define MONTH		4       //��
#define DAY  		3       //��
#define HOUR		2       //ʱ
#define MINUTE		1       //��
#define SECOND		0       //��

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
