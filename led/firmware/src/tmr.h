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

#ifndef _TMR_H    /* Guard against multiple inclusion */
#define _TMR_H
#include <stdint.h>
volatile int i;
extern volatile uint32_t msCount;

void WDTtimer2Init();
void delayN10ms(uint8_t msTime);


#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
