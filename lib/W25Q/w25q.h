#ifndef _W25Q_H_
#define _W25Q_H_

#include <Arduino.h>
#include <SPI.h>

#define W25QDEBUG
#ifdef  W25QDEBUG
#define DEBUG(...) {Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
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
class W25Q {
    public:
        W25Q(uint8_t sckPin = 14, uint8_t misoPin = 12, uint8_t mosiPin = 13, uint8_t csPin = 15);

        bool begin(uint32_t freq = 0);

        uint16_t readManufacterAndDeviceID(void);
        String readUniqueID(void);
        
    private:
        SPIClass *_w25q_spi;
        uint8_t _sck, _miso, _mosi, _cs;
};




#endif



