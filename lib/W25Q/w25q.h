#ifndef _W25Q_H_
#define _W25Q_H_

#include <Arduino.h>
#include <SPI.h>

#define W25QDEBUG
#ifdef  W25QDEBUG

#define DEBUG(...) { \
    Serial.print("["); \
    Serial.print(__FUNCTION__); \
    Serial.print("(): "); \
    Serial.print(__LINE__); \
    Serial.print("] "); \
    Serial.printf(__VA_ARGS__); \
    Serial.println(); \
}

#else
#define DEBUG(...)
#endif

#define W25Q_CHIP_CS_HIGH     digitalWrite(_cs, HIGH)
#define W25Q_CHIP_CS_LOW      digitalWrite(_cs, LOW)





/*
 *  Manufacturer and Device Identification
 */  
#define MANUFACTURER_ID         0xEF 



#define W25Q_CMD_READ_ID            0x4B
#define W25Q_CMD_READ_MDEVICEID     0x90
#define W25Q_CMD_READ_DEVICEID      0xAB
#define W25Q_CMD_READ_JEDEC_ID      0x9F

#define W25Q_CMD_NON_WRITE_ENABLE   0x06 
#define W25Q_CMD_WRITE_ENABLE       0x50
#define W25Q_CMD_WRITE_DISABLE      0x04

#define W25Q_CMD_READ_SR1           0x05
#define W25Q_CMD_READ_SR2           0x35
#define W25Q_CMD_READ_SR3           0x15
#define W25Q_CMD_WRITE_SR1          0x01
#define W25Q_CMD_WRITE_SR2          0x31
#define W25Q_CMD_WRITE_SR3          0x11

#define W25Q_CMD_READ_FLASH         0x03

#define W25Q_CMD_PAGE_PROGRAM       0x02

#define W25Q_CMD_CHIP_ERASE         0xC7
#define W25Q_CMD_SECTOR_ERASE       0x20
#define W25Q_CMD_BLOCK32_ERASE      0x52
#define W25Q_CMD_BLOCK64_ERASE      0xD8

#define W25Q_CMD_SUSPEND            0x75
#define W25Q_CMD_RESUME             0x7A
#define W25Q_CMD_POWERDOWN          0xB9
#define W25Q_CMD_POWERDOWN_RELEASE  0xAB
#define W25Q_CMD_RESET_ENABLE       0x66
#define W25Q_CMD_RESET              0x99


typedef enum {
    W25Q10_ID = 0x10,
    W25Q20_ID = 0x11,
    W25Q40_ID = 0x12,
    W25Q80_ID = 0x13,
    W25Q16_ID = 0x14,
    W25Q32_ID = 0x15,
    W25Q64_ID = 0x16,
    W25Q128_ID = 0x17,
    W25Q256_ID = 0x18,
    W25Q512_ID = 0x19,
} W25QName_t;

typedef enum{
    eNonVolMode = 0,
    eVolMode,
} eWEMode_t;
class W25Q {
    public:
        typedef struct {
            String      unique_id;
            uint16_t    manufacter_and_device_id;
            W25QName_t  device_id;
        } eID_t;

        typedef struct {
            uint32_t page_size;          
            uint32_t sector_size;       
            uint32_t block_size;         
            uint32_t sector_number;     
            uint32_t block_number;     
            uint32_t addr_start;         
            uint32_t addr_end;           
        } __attribute__ ((packed)) eInfo_t;
        
        typedef enum {
            eStatusReg1 = 0,
            eStatusReg2,
            eStatusReg3,
        } eSR_t;

        typedef struct {
            uint8_t busy : 1; /*!< Erase/Write in Progress*/
            uint8_t wel : 1; /*!< Write Enable Latch */
            uint8_t bp0 : 1; /*!< Block Protect Bits(non-volatile) */
            uint8_t bp1 : 1; /*!< Block Protect Bits(non-volatile) */
            uint8_t bp2 : 1; /*!< Block Protect Bits(non-volatile) */
            uint8_t tb : 1; /*!< Top/Bottom Protect(non-volatile) */
            uint8_t sec : 1; /*!< Sector Protect(non-volatile) */
            uint8_t srp : 1; /*!< Status Register Protect(non-volatile) */
        } __attribute__ ((packed)) sSR1_t; 

          typedef struct{
            uint8_t srl : 1; /*!< Status Register Lock(Volatile/Non-Volatile Writable)*/
            uint8_t qe : 1; /*!< Quad Enable(Volatile/Non-Volatile Writable)  */
            uint8_t r : 1; /*!< Reserved */
            uint8_t lb1 : 1; /*!< Security Register Lock Bits(Volatile/Non-Volatile OTP Writable) */
            uint8_t lb2 : 1; /*!< Security Register Lock Bits(Volatile/Non-Volatile OTP Writable) */
            uint8_t lb3 : 1; /*!< Security Register Lock Bits(Volatile/Non-Volatile OTP Writable) */
            uint8_t cmp : 1; /*!< Complement Protect(Volatile/Non-Volatile Writable) */
            uint8_t sus : 1; /*!< Suspend Status(Status-only) */
        } __attribute__ ((packed)) sSR2_t;
        
          typedef struct{
            uint8_t r1 : 1; /*!< Reserved*/
            uint8_t r2 : 1; /*!< Output Driver Strength(Volatile/Non-Volatile Writable)  */
            uint8_t wps : 1; /*!< Output Driver Strength(Volatile/Non-Volatile Writable) */
            uint8_t r3 : 1; /*!< Reserved */
            uint8_t r4 : 1; /*!< Reserved */
            uint8_t drv2 : 1; /*!< Write Protect Selection(Volatile/Non-Volatile Writable) */
            uint8_t drv1 : 1; /*!< Reserved */
            uint8_t r5 : 1; /*!< Reserved */
        } __attribute__ ((packed)) sSR3_t;

        W25Q(uint8_t sckPin = 14, uint8_t misoPin = 12, uint8_t mosiPin = 13, uint8_t csPin = 15);

        bool        begin(uint32_t freq = 0);

        uint16_t    readManufacterAndDeviceID(void);
        String      readUniqueID(void);
        W25QName_t  readDeviceID(void);
        uint32_t    readFlashCapacityByte(void);
        bool        writeEnable(eWEMode_t mode);
        void        writeDisable(void);
        void        writeSR(eSR_t sr, uint8_t srVal, eWEMode_t mode);
        uint8_t     readSR(eSR_t sr);
        void        readFlash(uint32_t addr, void *pBuf, uint16_t len);
        void        writeFlash(uint32_t addr, void *pBuf, uint16_t len);
        void        eraseChip(void);
        void        eraseSector(uint32_t addr, bool isFirstAddr);
        void        eraseBlock32(uint32_t addr, bool isFirstAddr);
        void        eraseBlock64(uint32_t addr, bool isFirstAddr);
        void        eraseOrProgramSuspend(void);
        void        eraseOrProgramResume(void);
        void        powerDown(void);
        void        releasePowerDown(void);
        void        reset(void);

        uint32_t    getSectorFirstAddr(uint32_t addr, uint16_t *secNum);
        uint32_t    getBlock32FirstAddr(uint32_t addr, uint16_t *b32Num);
        uint32_t    getBlock64FirstAddr(uint32_t addr, uint16_t *b64Num);


        bool        waitBusy(void);
    private:
        SPIClass    *_spi;
        eID_t       _id;
        eInfo_t     _info;

        uint8_t     _sck, _miso, _mosi, _cs;

};
        





#endif



