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

bool system_get_rtc_time(void);
void system_set_rtc_time(uint8_t *new_time);
    
    
    
    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
