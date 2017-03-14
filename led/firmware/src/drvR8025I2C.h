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

#ifndef _DRVR8025I2C_H    /* Guard against multiple inclusion */
#define _DRVR8025I2C_H

#include "app.h"
#include "../include/stdbool.h"
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

    
void drv_I2C_init(void);
bool drv_R8025T_init(void);
uint8_t drv_R8025T_read(uint8_t addr,uint8_t* data,uint8_t len);
bool drv_R8025T_write(uint8_t addr,uint8_t* data,uint8_t len);   
    
    
    
    
    
    
    
    

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
