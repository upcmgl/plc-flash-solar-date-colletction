/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _OSTIMER_H    /* Guard against multiple inclusion */
#define _OSTIMER_H

#include "drvR8025I2C.h"
#include "drv_i2c.h"
#include "../include/stdbool.h"
#include "../include/stdint.h"
#include "baseFun.h"

//define the describe of one day,DayStart index the start of one day,DayEnd index the end
//of one day.
#define DayStart      1
#define DayEnd        2


struct timeDateStr
{
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
};
struct sysTimeStr
{
   struct  timeDateStr timeDate; 
   uint8_t sysTimeRunState;
};

extern  struct sysTimeStr sysTime;

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */


/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t initialTime[];
bool system_get_rtc_time(void);
void system_set_rtc_time(uint8_t *new_time);
void secInterrupt(uint8_t secs);    
void minInterrupt(uint8_t mins);  
void setAlarmClockStart(uint8_t clock);
    
    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
