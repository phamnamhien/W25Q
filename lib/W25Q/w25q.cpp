#include "w25q.h"



#define DEFAULT_SPI_CS_PIN      11
#define DEFAULT_HSPI_CS_PIN     15
#define DEFAULT_VSPI_CS_PIN     5


W25Q::W25Q(uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin, uint8_t csPin) {
    _sck = sckPin;
    _miso = misoPin;
    _mosi = mosiPin;
    _cs = csPin;
}


bool W25Q::begin(uint32_t freq) {
    if (_spi) {
        delete _spi;
        DEBUG("Uninstall SPI Driver.");
    }
    _spi = new SPIClass(SPI);
    DEBUG("Create new SPI Driver.");
    if (_spi == NULL) {
        DEBUG("Failed to create new SPI Driver.");
        return false;
    }
    DEBUG("Install SPI Driver.");
    _spi->begin(_sck, _miso, _mosi, _cs);
    if (freq) {
        DEBUG("Set SPI frequency is %d", freq);
        _spi->beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    }    
    pinMode(_cs, OUTPUT);

    _id.manufacter_and_device_id = readManufacterAndDeviceID();
    DEBUG("Manualfacter and device ID: 0x%04X", _id.manufacter_and_device_id);
    _id.device_id = readDeviceID();
    DEBUG("Device ID: 0x%02X", _id.device_id);
    _id.unique_id = readUniqueID();
    DEBUG("Unique ID: %s", _id.unique_id.c_str());
    
    _info.addr_start = 0x000000;
    DEBUG("Start address: 0x%08X", _info.addr_start);
    _info.addr_end = readFlashCapacityByte() - 1;
    DEBUG("End address: 0x%08X", _info.addr_end);
    _info.page_size = 256;
    DEBUG("Page size: %d", _info.page_size);
    _info.sector_size = 4096;
    DEBUG("Sector size: %d", _info.sector_size);
    _info.block_size = 65536;
    DEBUG("Block size: %d", _info.block_size);
    _info.sector_number = (_info.addr_end + 1 - _info.addr_start)  / _info.sector_size;
    DEBUG("Sector number: %d", _info.sector_number);
    _info.block_number = (_info.addr_end + 1 - _info.addr_start)  / _info.block_size;
    DEBUG("Block number: %d", _info.block_number);
       

    return true;
}

String W25Q::readUniqueID(void) {
    uint8_t temp;
    char cBuf[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    String id = "";
  
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_READ_ID);
     _spi->transfer(0xFF);
    _spi->transfer(0xFF);
    _spi->transfer(0xFF);
    _spi->transfer(0xFF);
    for (uint8_t i = 0; i < 8; i++) {
        temp = _spi->transfer(0xFF);
        // DBG(temp, HEX);
        id += cBuf[(temp>>4) & 0x0F];
        id += cBuf[temp & 0x0F];
    }
    W25Q_CHIP_CS_HIGH;
    return id;
}

uint16_t W25Q::readManufacterAndDeviceID(void) {
    uint16_t val;
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_READ_MDEVICEID);
    _spi->transfer(0x00);
    _spi->transfer(0x00);
    _spi->transfer(0x00);
    val = _spi->transfer(0xFF) << 8;
    val |= _spi->transfer(0xFF);
    W25Q_CHIP_CS_HIGH;
    return val;
}
W25QName_t W25Q::readDeviceID(void) {
    uint8_t val;
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_READ_DEVICEID);
    _spi->transfer(0xFF);
    _spi->transfer(0xFF);
    _spi->transfer(0xFF);
    val = _spi->transfer(0xFF);
    W25Q_CHIP_CS_HIGH;
    return (W25QName_t)val;
}
uint32_t W25Q::readFlashCapacityByte(void) {
    uint32_t val;
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_READ_JEDEC_ID);
    _spi->transfer(0xFF);
    _spi->transfer(0xFF);
    val = pow(2, _spi->transfer(0xFF));
    W25Q_CHIP_CS_HIGH;
    return val;
}

bool W25Q::writeEnable(eWEMode_t mode) {
    uint8_t val, cmd;
    switch ((uint8_t)mode) {
        case eNonVolMode:
            cmd = W25Q_CMD_NON_WRITE_ENABLE;
            break;
        case eVolMode:
            cmd = W25Q_CMD_WRITE_ENABLE;
            break;
    }
    W25Q_CHIP_CS_LOW;
    _spi->transfer(cmd);
    W25Q_CHIP_CS_HIGH;
    val = readSR(eStatusReg1);
    if (((sSR1_t *)(&val))->wel) return true;
    else return false;
}

void W25Q::writeDisable(void) {
    W25Q_CHIP_CS_HIGH;
    _spi->transfer(W25Q_CMD_WRITE_DISABLE);
    W25Q_CHIP_CS_LOW;
}
uint8_t W25Q::readSR(eSR_t sr) {
    uint8_t command = 0,val = 0;
    switch ((uint8_t)sr) {
        case eStatusReg1: 
            command = W25Q_CMD_READ_SR1;
            break;
        case eStatusReg2: 
            command = W25Q_CMD_READ_SR2;
            break;
        case eStatusReg3: 
            command = W25Q_CMD_READ_SR3;
            break;
        default:
            command = W25Q_CMD_READ_SR1;
            break;
    }
    W25Q_CHIP_CS_LOW;
    _spi->transfer(command);
    val = _spi->transfer(0xFF);
    // DBG(val);
    W25Q_CHIP_CS_HIGH;
    return val;
}
void W25Q::writeSR(eSR_t sr, uint8_t srVal, eWEMode_t mode) {
    uint8_t command = 0;
    switch ((uint8_t)sr) {
        case eStatusReg1: 
            command = W25Q_CMD_WRITE_SR1;
            break;
        case eStatusReg2: 
            command = W25Q_CMD_WRITE_SR2;
            break;
        case eStatusReg3: 
            command = W25Q_CMD_WRITE_SR3;
            break;
        default:
            command = W25Q_CMD_WRITE_SR1;
            break;
    }
    while (waitBusy());
    writeEnable(mode);
    W25Q_CHIP_CS_LOW;
    _spi->transfer(command);
    _spi->transfer(srVal);
    W25Q_CHIP_CS_HIGH;
}

void W25Q::readFlash(uint32_t addr, void *pBuf, uint16_t len) {
    if ((addr > 0xFFFFFF)||(pBuf == NULL)||(len == 0)) return;
    uint8_t *buf = (uint8_t *)pBuf;
    while (waitBusy());
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_READ_FLASH);
    _spi->transfer((uint8_t)(addr>>16));
    _spi->transfer((uint8_t)(addr>>8));
    _spi->transfer((uint8_t)(addr));
    for (uint16_t i = 0; i < len; i++) {
        buf[i] = _spi->transfer(0xFF);
    }
    W25Q_CHIP_CS_HIGH;
}
void W25Q::writeFlash(uint32_t addr, void *pBuf, uint16_t len) {
    if ((addr > _info.addr_end) || (pBuf == NULL) || (len == 0)) return;
    uint8_t *buf = (uint8_t *)pBuf;
    uint16_t left = len;
    while (left) {
        left > _info.page_size ? len = _info.page_size : len = left;
        while (waitBusy());
        writeEnable(eNonVolMode);
        W25Q_CHIP_CS_LOW;
        _spi->transfer(W25Q_CMD_PAGE_PROGRAM);
        _spi->transfer((uint8_t)(addr>>16));
        _spi->transfer((uint8_t)(addr>>8));
        _spi->transfer((uint8_t)(addr));
        for (uint16_t i = 0; i < len; i++) {
            _spi->transfer(buf[i]);
        }
        W25Q_CHIP_CS_HIGH;
        buf += len;
        left -= len;
        addr += len;
    }
}

void W25Q::eraseChip(void) {
    while (waitBusy());
    writeEnable(eNonVolMode);
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_CHIP_ERASE);
    W25Q_CHIP_CS_HIGH;
}

void W25Q::eraseSector(uint32_t addr, bool isFirstAddr) {
    if ((addr > _info.addr_end)||(isFirstAddr && (!(addr%_info.sector_size))))  {
        return;
    }
    while (waitBusy());
    while (!writeEnable(eNonVolMode));
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_SECTOR_ERASE);
    _spi->transfer((uint8_t)(addr>>16));
    _spi->transfer((uint8_t)(addr>>8));
    _spi->transfer((uint8_t)(addr));
    W25Q_CHIP_CS_HIGH;
}

void W25Q::eraseBlock32(uint32_t addr, bool isFirstAddr) {
    if ((addr > _info.addr_end)||(isFirstAddr && (!(addr%(_info.block_size/2))))) {
        return;
    }
    while (waitBusy());
    while (!writeEnable(eNonVolMode));
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_BLOCK32_ERASE);
    _spi->transfer((uint8_t)(addr>>16));
    _spi->transfer((uint8_t)(addr>>8));
    _spi->transfer((uint8_t)(addr));
    W25Q_CHIP_CS_HIGH;
}

void W25Q::eraseBlock64(uint32_t addr, bool isFirstAddr) {
    if((addr > _info.addr_end)||(isFirstAddr && (!(addr%_info.block_size)))) {
        return;
    }
    while (waitBusy());
    while (!writeEnable(eNonVolMode));
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_BLOCK64_ERASE);
    _spi->transfer((uint8_t)(addr>>16));
    _spi->transfer((uint8_t)(addr>>8));
    _spi->transfer((uint8_t)(addr));
    W25Q_CHIP_CS_HIGH;
}


void W25Q::eraseOrProgramSuspend(void) {
    if ((readSR(eStatusReg2)&0x80) || (!waitBusy())) {
        return;
    }    
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_SUSPEND);
    W25Q_CHIP_CS_HIGH;
    delay(1);
}
void W25Q::eraseOrProgramResume(void) {
    if((!(readSR(eStatusReg2)&0x80)) || (waitBusy())) {
        return;
    }    
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_RESUME);
    W25Q_CHIP_CS_HIGH;
}

void W25Q::powerDown(void) {
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_POWERDOWN);
    W25Q_CHIP_CS_HIGH;
}

void W25Q::releasePowerDown(void) {
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_POWERDOWN_RELEASE);
    W25Q_CHIP_CS_HIGH;
}

void W25Q::reset(void) {
    while (waitBusy());
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_RESET_ENABLE);
    _spi->transfer(W25Q_CMD_RESET);
    W25Q_CHIP_CS_HIGH;
    delay(1);
}

uint32_t W25Q::getSectorFirstAddr(uint32_t addr, uint16_t *secNum) {
    uint16_t num;
    if (addr > _info.addr_end) return addr;
    num = addr / _info.sector_size;
    if (secNum != NULL) *secNum = num;
    return (uint32_t)(num * _info.sector_size);
}

uint32_t W25Q::getBlock32FirstAddr(uint32_t addr, uint16_t *b32Num) {
  if (addr > _info.addr_end) return addr;
  uint16_t num;
  num = addr / (_info.block_size / 2);
  if (b32Num != NULL) *b32Num = num;
  return (uint32_t)(num * (_info.block_size/2));
}

uint32_t W25Q::getBlock64FirstAddr(uint32_t addr, uint16_t *b64Num) {
    if (addr > _info.addr_end) return addr;
    uint16_t num;
    num = addr / _info.block_size;
    if (b64Num != NULL) *b64Num = num;
    return (uint32_t)(num * _info.block_size);
}




bool W25Q::waitBusy(void) {
    uint8_t temp;
    W25Q_CHIP_CS_LOW;
    _spi->transfer(W25Q_CMD_READ_SR1);
    temp = _spi->transfer(0xFF);
    W25Q_CHIP_CS_HIGH;
    if (temp & 0x01) return true;
    else return false;
}
