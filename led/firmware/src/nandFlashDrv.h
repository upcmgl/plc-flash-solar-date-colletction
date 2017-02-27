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

#ifndef _NANDFLASHDRV_H    /* Guard against multiple inclusion */
#define _NANDFLASHDRV_H

#include "app.h"

struct nanddrv_transfer {
	unsigned char *buffer;
	int offset;
	int nbytes;
};
struct nand_chip {
	int blocks;
	int pages_per_block;
	int data_bytes_per_page;
	int spare_bytes_per_page;
	int bus_width_shift;
};


/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#define NAND_MX30LF1G08AA     0x1D80F1C2
#define NAND_FLASH_ID         NAND_MX30LF1G08AA

#define NAND_FLASH_BLOCK_SIZE_PAGE  (64) // 64K Page(2048+64)
#define NAND_FLASH_PAGE_SIZE        (2048)      // 2KB/page
#define NAND_FLASH_OOB_SIZE         (64)
#define NAND_FLASH_BLOCK_SIZE     (NAND_FLASH_BLOCK_SIZE_PAGE * NAND_FLASH_PAGE_SIZE * 1024UL)
#define NAND_FLASH_BLOCK_COUNT   (1024)

#define NAND_PAGE_SMALL_LARGE false

//#define NAND_CE_PORT     PORT_CHANNEL_A
#define NAND_CE_PIN      PORTGbits.RG6
#define NAND_CE_PIN_LOW    (PLIB_PORTS_PinClear(PORTS_ID_0,PORT_CHANNEL_A, PORTS_BIT_POS_5))//PORTACLR = 0x0020
#define NAND_CE_PIN_HIGH   (PLIB_PORTS_PinSet(PORTS_ID_0,PORT_CHANNEL_A, PORTS_BIT_POS_5))//PORTASET = 0x0020
//#define NAND_CE_PIN_HIGH (PORTCbits.w=PORTAbits.w |0x0020)
//#define NAND_CE_PIN_LOW (PORTCbits.w=PORTAbits.w & (~0x0020))
#ifdef WAIT_REDAY_USE_PIN
//#define NAND_RDY_PORT    PORT_CHANNEL_
//#define NAND_RDY_PIN     PORTS_BIT_POS_
#endif

//#define NAND_WP_PORT     PORT_CHANNEL_G
#define NAND_WP_PIN      PORTCbits.RC1
#define NAND_WP_PIN_LOW    (PLIB_PORTS_PinClear(PORTS_ID_0,PORT_CHANNEL_C, PORTS_BIT_POS_1))//PORTGCLR = 0x8000
#define NAND_WP_PIN_HIGH   (PLIB_PORTS_PinSet(PORTS_ID_0,PORT_CHANNEL_C, PORTS_BIT_POS_1))//PORTGSET = 0x8000
//#define NAND_WP_PIN_HIGH (PORTCbits.w=PORTGbits.w | 0x8000)
//#define NAND_WP_PIN_LOW (PORTCbits.w=PORTGbits.w & (~0x8000))
//#define NAND_ALE_ADDR_POS  EBI_ADDR_POS_12
//#define NAND_ALE_PORT     PORT_CHANNEL_C
#define NAND_ALE_PIN      PORTCbits.RC3
#define NAND_ALE_PIN_LOW    (PLIB_PORTS_PinClear(PORTS_ID_0,PORT_CHANNEL_C, PORTS_BIT_POS_3))//PORTCCLR = 0x0004
#define NAND_ALE_PIN_HIGH   (PLIB_PORTS_PinSet(PORTS_ID_0,PORT_CHANNEL_C, PORTS_BIT_POS_3))//PORTCSET = 0x0004
//#define NAND_ALE_PIN_HIGH (PORTCbits.w=PORTCbits.w | 0x0004)
//#define NAND_ALE_PIN_LOW (PORTCbits.w=PORTCbits.w & (~0x0004))
//#define NAND_CLE_ADDR_POS  EBI_ADDR_POS_6
//#define NAND_CLE_PORT     PORT_CHANNEL_C
#define NAND_CLE_PIN      PORTCbits.RC4
#define NAND_CLE_PIN_LOW    (PLIB_PORTS_PinClear(PORTS_ID_0,PORT_CHANNEL_C, PORTS_BIT_POS_4))//PORTCCLR = 0x0002
#define NAND_CLE_PIN_HIGH   (PLIB_PORTS_PinSet(PORTS_ID_0,PORT_CHANNEL_C, PORTS_BIT_POS_4))//PORTCSET = 0x0002    
//#define NAND_CLE_PIN_HIGH PORTCbits.w=PORTCbits.w | 0x0002)
//#define NAND_CLE_PIN_LOW  (PORTCbits.w=PORTCbits.w & (~0x0002))

//#define NAND_WE_PORT      PORT_CHANNEL_C
#define NAND_WE_PIN       PORTCbits.RC2
#define NAND_WE_PIN_LOW    (PLIB_PORTS_PinClear(PORTS_ID_0,PORT_CHANNEL_C, PORTS_BIT_POS_2))//PORTCCLR = 0x0008
#define NAND_WE_PIN_HIGH   (PLIB_PORTS_PinSet(PORTS_ID_0,PORT_CHANNEL_C, PORTS_BIT_POS_2))//PORTCSET = 0x0008    
//#define NAND_WE_PIN_HIGH (PORTCbits.w=PORTCbits.w | 0x0008)
//#define NAND_WE_PIN_LOW (PORTCbits.w=PORTCbits.w &(~0x0008))

//#define NAND_RE_PORT      PORT_CHANNEL_C
#define NAND_RE_PIN     PORTGbits.RG7
#define NAND_RE_PIN_LOW   (PLIB_PORTS_PinClear(PORTS_ID_0,PORT_CHANNEL_G, PORTS_BIT_POS_7))// PORTCCLR = 0x0010
#define NAND_RE_PIN_HIGH  (PLIB_PORTS_PinSet(PORTS_ID_0,PORT_CHANNEL_G, PORTS_BIT_POS_7))// PORTCSET = 0x0010   
//#define NAND_RE_PIN_HIGH (PORTCbits.w=PORTCbits.w | 0x0010)
//#define NAND_RE_PIN_LOW (PORTCbits.w=PORTCbits.w & (~0x0010))
//    
//#define NAND_CMD_EBI_ADDR  (SRAM_ADDR_CS0+(1UL<<NAND_CLE_ADDR_POS))
//#define NAND_ADDR_EBI_ADDR (SRAM_ADDR_CS0+(1UL<<NAND_ALE_ADDR_POS))
//#define NAND_DATA_EBI_ADDR (SRAM_ADDR_CS0)


extern volatile uint8_t DataIn;   
void DRV_NAND_FLASH_Initialize(void);
void __attribute__((optimize ("O0"))) NandFlashRaw_Reset(void);
uint8_t __attribute__((optimize ("O0"))) NandFlash_ReadStatus(void);

uint32_t __attribute__((optimize ("O0"))) NandFlashRaw_ReadId(void);
uint8_t NandFlashRaw_EraseBlock(uint16_t block);
uint8_t __attribute__((optimize ("O0"))) RawNandFlash_ReadPage(uint16_t block,  uint16_t page_in_block, uint16_t page_offset,   void *data, uint16_t read_len);
//uint8_t __attribute__((optimize ("O0"))) NandFlashRaw_WritePage(uint16_t block, uint16_t page_in_block, uint16_t page_offset,void *data, uint16_t data_len);


void ControlPortInit();
void SetPortEDirectionOut();
void SetPortEDirectionIn();
void  WRITE_COMMAND(uint8_t command);
void __attribute__((optimize ("O3")))WriteColumnAddress(uint16_t columnAddress);
void __attribute__((optimize ("O3")))WriteRowAddress(uint32_t rowAddress);
uint32_t __attribute__((optimize ("O0")))NandFlashRaw_ReadId(void);
//uint8_t __attribute__((optimize ("O0")))NandFlashRaw_Status(void);---->static
//void __attribute__((optimize ("O0")))WaitReady();----->>>>>static
void __attribute__((optimize("O3"))) WriteData(uint8_t *buffer,uint32_t size);
void __attribute__((optimize("O3"))) ReadData(uint8_t *buffer,uint32_t size);
void __attribute__((optimize ("O0")))NandFlashRest();
void __attribute__((optimize ("O3")))NandFlashWriteReadStartAddress(uint16_t rowAddress);
void __attribute__((optimize ("O0"))) NandFlashWriteReadOffsetAddress(uint16_t page_offset);
uint8_t __attribute__((optimize ("O0"))) EraseBlock(uint16_t block);
void __attribute__((optimize ("O0")))NandFlashReadDemo(uint8_t *buffer1,uint32_t size );
uint8_t __attribute__((optimize ("O0"))) NandFlashRaw_WritePage(uint16_t block, uint16_t page_in_block, uint16_t page_offset,void *data, uint16_t data_len);
    /* Provide C++ Compatibility */

uint8_t __attribute__((optimize ("O0"))) nanddrv_read_tr( int page,struct nanddrv_transfer * ,uint8_t n_tr);
uint8_t __attribute__((optimize ("O0"))) nanddrv_write_tr( int page,struct nanddrv_transfer * ,uint8_t n_tr);
uint8_t __attribute__((optimize ("O0")))nanddrv_erase( int block); 


    
    
    


    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
