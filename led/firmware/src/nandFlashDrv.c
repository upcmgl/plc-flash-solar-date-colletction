#include "nandFlashDrv.h"
/*
 * /*----------------------------------------------------------------------------
 *        Internal definitions
 *----------------------------------------------------------------------------*/

/** Nand flash chip status codes*/
#define STATUS_READY                    (1 << 6)
#define STATUS_ERROR                    (1 << 0)

/** Nand flash commands*/
#define COMMAND_READ_1                  0x00
#define COMMAND_READ_2                  0x30
#define COMMAND_COPYBACK_READ_1         0x00
#define COMMAND_COPYBACK_READ_2         0x35
#define COMMAND_COPYBACK_PROGRAM_1      0x85
#define COMMAND_COPYBACK_PROGRAM_2      0x10
#define COMMAND_RANDOM_OUT              0x05
#define COMMAND_RANDOM_OUT_2            0xE0
#define COMMAND_RANDOM_IN               0x85
#define COMMAND_READID                  0x90
#define COMMAND_WRITE_1                 0x80
#define COMMAND_WRITE_2                 0x10
#define COMMAND_ERASE_1                 0x60
#define COMMAND_ERASE_2                 0xD0
#define COMMAND_STATUS                  0x70
#define COMMAND_RESET                   0xFF


/** Nand flash commands (small blocks)*/
#define COMMAND_READ_A                  0x00
#define COMMAND_READ_C                  0x50

volatile uint8_t DataIn; //用于PIC读数据8位数据
/**/
#define ENABLE_CE() {NAND_CE_PIN=0;}
#define DISABLE_CE() {NAND_CE_PIN=1;}

uint8_t PageReadComplete=0;  //this flag is for page read complete;
void SetPortEDirectionOut(){
   TRISEbits.w=TRISEbits.w&0xFF00;    //set PORT to outpin
   ODCEbits.w=ODCEbits.w&0x00FF;    
}
void SetPortEDirectionIn(){
    TRISEbits.w=TRISEbits.w|0xff;    //set PORT to Input
    ODCEbits.w=ODCEbits.w&0xff00;    //
}
//#define WRITE_COMMAND(command) {PORTEbits.w=(PORTEbits.w&0xff00)|(uint8_t)command;}
/*
 * \brief write command to portE
 */
void  __attribute__((optimize ("O0")))  WRITE_COMMAND(uint8_t command) 
{
    SetPortEDirectionOut();
    NAND_CE_PIN_LOW;
    NAND_WE_PIN_HIGH;
    NAND_ALE_PIN_LOW;
    NAND_RE_PIN_HIGH;
    Nop();
    NAND_CLE_PIN_HIGH;
    Nop();
    NAND_WE_PIN_LOW;
    PORTEbits.w=(PORTEbits.w&0xff00)|(uint8_t)command;
    Nop();
    NAND_WE_PIN_HIGH;
    Nop();
    NAND_CLE_PIN_LOW;
}
/*
 * \brief write address to portE
 */
void __attribute__((optimize ("O0")))  WRITE_ADDRESS(uint8_t address){
    SetPortEDirectionOut();
    NAND_CE_PIN_LOW;
    NAND_CLE_PIN_LOW;
    NAND_RE_PIN_HIGH;
    NAND_ALE_PIN_HIGH;
    Nop();
    NAND_WE_PIN_LOW;
    PORTEbits.w=(PORTEbits.w&0xff00)|(uint8_t)address;
    Nop();
    NAND_WE_PIN_HIGH;
    Nop();
    
}

/*
 * \brief read the 8bits data from PortE
 */
static uint8_t  __attribute__((optimize ("O0")))  READ_DATA8(){
    SetPortEDirectionIn();
    NAND_CE_PIN_LOW;
    NAND_CLE_PIN_LOW;
    NAND_ALE_PIN_LOW;
    NAND_RE_PIN_HIGH;
    NAND_WE_PIN_HIGH;
    Nop();
    Nop();
    NAND_RE_PIN_LOW;
    Nop();
    Nop();
    NAND_RE_PIN_HIGH;
    Nop();
    DataIn=PORTEbits.w&0xFF;
    Nop();
    return DataIn;
}
static void  __attribute__((optimize ("O0")))  WRITE_DATA8(uint8_t data){
    SetPortEDirectionOut();
    NAND_CE_PIN_LOW;
    NAND_CLE_PIN_LOW;
    NAND_ALE_PIN_LOW;
    NAND_RE_PIN_HIGH;
    NAND_WE_PIN_LOW;
    Nop();
    PORTEbits.w=(PORTEbits.w&0x00)|data;
    Nop();
    NAND_WE_PIN_HIGH;
    Nop();
}
static void __attribute__((optimize ("O0"))) Delay_ns(uint32_t ns)
{
    uint32_t start_tick;
    uint32_t timeout_tick;
    uint32_t SYS_FREQUENCY=80000000;
    asm volatile("mfc0   %0, $9" : "=r"(start_tick));
    timeout_tick = ns/(1000000000/(SYS_FREQUENCY/2));
//    timeout_tick = ns/10;
    while((_CP0_GET_COUNT()-start_tick)<timeout_tick);
}
/**
 * \brief Sends the column address to the NandFlash chip.
 *
 * \param columnAddress  Column address to send.
 */
void __attribute__((optimize ("O0")))WriteColumnAddress(uint16_t columnAddress)
{
   
    //send A7-A0
    WRITE_ADDRESS(columnAddress);
    //send A11-A8
    WRITE_ADDRESS((columnAddress >>8) & 0x0F);
    
}
/**
 * \brief Sends the row address to the NandFlash chip.
 *
 * \param  Row  address to send.
 *
 * 
 */
void __attribute__((optimize ("O0")))WriteRowAddress(uint32_t rowAddress)
{
    //send A19-12
    WRITE_ADDRESS(rowAddress);
    //send A27-20
    WRITE_ADDRESS((rowAddress >> 8));
}
/**
 * \brief Read data from the Nandflash chip into provided buffer.
 * 
 * \param buffer  Buffer where the data will be stored
 * \param size  Number of bytes that will be written
 */
void __attribute__((optimize("O0"))) ReadData(uint8_t *buffer1,uint32_t size){
    unsigned int i=0;
    for(i=0;i<size;i++){
        buffer1[i]=READ_DATA8();
    }
}
void __attribute__((optimize("O3"))) WriteData(uint8_t *buffer,uint32_t size){
    unsigned int i=0;
    Nop();
    for(i=0;i<size;i++){
        WRITE_DATA8(buffer[i]);
    }
}
/*
 * \brief Read the NandFlash's status use the command  0x70h
 */
static uint8_t __attribute__((optimize ("O0")))NandFlashRaw_Status(void){
    uint8_t Status;

    WRITE_COMMAND(0x70);
    Delay_ns(90);
    Status  = READ_DATA8();
    return Status;
}
static void __attribute__((optimize ("O0")))WaitReady(){

    /*-------------------watch the wait ready function------------------------------*/
     WRITE_COMMAND(COMMAND_STATUS);
    // tWHR >60ns
    Delay_ns(90);
    while ((READ_DATA8() & STATUS_READY) != STATUS_READY);
    
    
}
/** 
 * \brief To write the read start address
 */
void __attribute__((optimize ("O0")))NandFlashWriteReadStartAddress(uint16_t rowAddress)
{
    WRITE_COMMAND(COMMAND_READ_1);  
    //send A7-A0
    WRITE_ADDRESS(0);  
    //send A11-A8
    WRITE_ADDRESS(0);
    //send A19-12
    WRITE_ADDRESS(rowAddress);
    //send A27-20
    WRITE_ADDRESS((rowAddress >> 8));
    Nop();
    NAND_ALE_PIN=0;
    WRITE_COMMAND(COMMAND_READ_2);
}

/**
 * \brief To write the read offset Address
 */
void __attribute__((optimize ("O0"))) NandFlashWriteReadOffsetAddress(uint16_t page_offset){
    Nop();
    WRITE_COMMAND(COMMAND_RANDOM_OUT);
     //send A7-A0
    WRITE_ADDRESS(page_offset);
    //send A11-A8
    WRITE_ADDRESS((page_offset >>8) & 0x0F);
    WRITE_COMMAND(COMMAND_RANDOM_OUT_2);

}
/**
 * \brief Erases the specified block of the device.
 *
 * \param block  Number of the physical block to erase.
 * \return 0 if successful; otherwise returns an error code.
 */
uint8_t __attribute__((optimize ("O0"))) EraseBlock(uint16_t block)
{
    uint8_t error = 0;
    uint32_t address;
    uint32_t EraseTmr=0;
    //DEBUG_PRINT("EraseBlock(%d)\r\n", block);

    /* Start erase*/
    error = READ_DATA8();
    NAND_WP_PIN_HIGH;
    NAND_CE_PIN_LOW;
    /* Calculate address used for erase */
    address = block * NAND_FLASH_BLOCK_SIZE_PAGE;

    WRITE_COMMAND(COMMAND_ERASE_1);
    WriteRowAddress(address);
    WRITE_COMMAND(COMMAND_ERASE_2);
    WaitReady();
    //#ifdef WAIT_REDAY_USE_PIN
    WRITE_COMMAND(COMMAND_STATUS);
    // tWHR >60ns
    Delay_ns(90);
//    EraseTmr=MsCount;
//    while(MsCount-EraseTmr<3);
   // #endif
    NAND_WP_PIN_LOW;
    Nop();
    error = READ_DATA8();
    NAND_CE_PIN_HIGH;
    NAND_WP_PIN_LOW;
  
    return (error&0x01);
}
/**
 * \brief this is a demo function for read cycle
 * @return 
 */
void __attribute__((optimize ("O0")))NandFlashReadDemo(uint8_t *buffer2,uint32_t size ){
    unsigned int j=0;
    SetPortEDirectionOut();
    WRITE_COMMAND(0x0);
    WRITE_ADDRESS(0x0);
    WRITE_ADDRESS(0x0);
    WRITE_ADDRESS(0x9);
    WRITE_ADDRESS(0x0);
    NAND_ALE_PIN_LOW;
    WRITE_COMMAND(0x30);
    WaitReady();
    NandFlashWriteReadOffsetAddress(0);
    Delay_ns(90);
    for(j=0;j<size;j++){
        buffer2[j]=READ_DATA8();
    } 
}
uint8_t __attribute__((optimize ("O0"))) NandFlashRaw_WritePage(uint16_t block, uint16_t page_in_block, uint16_t page_offset,void *data, uint16_t data_len){
    uint8_t error = 0;
    uint8_t rowAddress;
    NAND_CE_PIN_LOW;
    NAND_WP_PIN_HIGH;
    rowAddress=block*NAND_FLASH_BLOCK_SIZE_PAGE+page_in_block;
    WRITE_COMMAND(COMMAND_WRITE_1);  //0x80
    WriteColumnAddress(0);
    WriteRowAddress(rowAddress);
    Delay_ns(120);
    if(page_offset)
    {
        WRITE_COMMAND(COMMAND_RANDOM_IN);  //0x85
        WriteColumnAddress(page_offset);
        //tADL = 100ns
        Delay_ns(120);
    }
    WriteData((uint8_t *) data, data_len);
    WRITE_COMMAND(COMMAND_WRITE_2);     //0x10
    
    WaitReady();
    /* Disable chip*/
    NAND_CE_PIN_HIGH;
    NAND_WP_PIN_LOW;

    return error;

}

uint32_t __attribute__((optimize ("O0")))NandFlashRaw_ReadId(void)
{
    uint32_t chipId;

    WRITE_COMMAND(COMMAND_READID);

    WRITE_ADDRESS(0);
    chipId  = READ_DATA8();
    chipId |= READ_DATA8() << 8;
    chipId |= READ_DATA8() << 16;
    chipId |= READ_DATA8() << 24;
    DISABLE_CE();
    return chipId;
}

void __attribute__((optimize ("O0")))NandFlashRest(){
     Nop();
     WRITE_COMMAND(0xFF);
     Nop();
}
/**
 *\brief this function is used for initial the control pins,such as wp#,ce#,cle,ale,we#,re#;all
 * of these pins are set digital output.
 */
void ControlPortInit(){
    /*set wp#*/
    TRISCbits.TRISC1=0;
    /*set ce*/
    TRISGbits.TRISG6=0;
    /*set cle*/
    TRISCbits.TRISC4=0;
    /*set ale*/
    TRISCbits.TRISC3=0;
    /*set we#*/
    TRISCbits.TRISC2=0;
    /*set re#*/
    TRISGbits.TRISG7=0;
    Nop();
}
static void nanddrv_send_addr(int page, int offset)
{
	if(offset >= 0){
        WRITE_ADDRESS(offset & 0xff);
        WRITE_ADDRESS((offset>>8) & 0x0f);
	}

	if(page >= 0){
        WRITE_ADDRESS(page & 0xff);
        WRITE_ADDRESS((page>>8) & 0xff);
	}
    NAND_ALE_PIN_LOW;
}
uint8_t  __attribute__((optimize ("O0"))) nanddrv_read_tr( int page,struct nanddrv_transfer *tr, uint8_t n_tr){
    int ncycles;
	if(n_tr < 1)
		return 0;
	WRITE_COMMAND( COMMAND_READ_1);  //0x00
	nanddrv_send_addr( page,tr->offset);
	WRITE_COMMAND( COMMAND_READ_2);  //0x30
	WaitReady();
    Delay_ns(90);
	WRITE_COMMAND(COMMAND_READ_2);   //0x30
	while (1) {
			unsigned char *buffer = tr->buffer;

			ncycles = tr->nbytes;
			while (ncycles> 0) {
				*buffer =  READ_DATA8();
				ncycles--;
				buffer++;
			}
		n_tr--;
		tr++;
		if(n_tr < 1)
			break;
		WRITE_COMMAND( COMMAND_RANDOM_OUT);   //0x05
		nanddrv_send_addr( -1, tr->offset);
		WRITE_COMMAND( COMMAND_RANDOM_OUT_2);  //0xE0
	}
	return 0;
}

/*
 * Program page
 * Cmd: 0x80, 5-byte address, data bytes,  Cmd: 0x10, wait not busy
 */
uint8_t  __attribute__((optimize ("O0"))) nanddrv_write_tr( int page,struct nanddrv_transfer *tr, uint8_t n_tr)
{
    uint8_t status;
    uint8_t *buffer;
    NAND_CE_PIN_LOW;
    NAND_WP_PIN_HIGH;
	int ncycles;
	if (n_tr < 1)
		return 0;
	WRITE_COMMAND(COMMAND_WRITE_1);  //0x80
	nanddrv_send_addr(page, tr->offset);
	while (1) {
			 buffer= tr->buffer;
			ncycles = tr->nbytes;
			while (ncycles> 0) {
                WRITE_DATA8( *buffer);
				ncycles--;
				buffer++;
			}
		n_tr--;
		tr++;
		if (n_tr < 1)
			break;
		WRITE_COMMAND(COMMAND_RANDOM_IN); //0x85
		nanddrv_send_addr(-1, tr->offset);
	}
    WRITE_COMMAND( COMMAND_WRITE_2); //0x10
    WaitReady();
    NAND_CE_PIN_HIGH;
    NAND_WP_PIN_LOW;
	status = NandFlashRaw_Status();
	if(status)
		return 0;
	return -1;
}

/*
 * Block erase
 * Cmd: 0x60, 3-byte address, cmd: 0xD0. Wait not busy.
 */
uint8_t  __attribute__((optimize ("O0"))) nanddrv_erase( int block)
{
	unsigned char status;

 	WRITE_COMMAND(COMMAND_ERASE_1);   //0x60
	nanddrv_send_addr(block * 64, -1);  //64==pages_per_block;
	WRITE_COMMAND( COMMAND_ERASE_2);     //0xD0  
	WaitReady();
	status = NandFlashRaw_Status();
	if(status)
		return 0;
	return -1;
}
