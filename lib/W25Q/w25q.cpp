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
    if (_w25q_spi) {
        delete _w25q_spi;
    }
    _w25q_spi = new SPIClass(SPI);
    if(_w25q_spi == NULL) {
        return false;
    }
    _w25q_spi->begin(_sck, _miso, _mosi, _cs);
    if (freq) {
        _w25q_spi->beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    }    
    pinMode(_cs, OUTPUT);
    return true;
}

String W25Q::readUniqueID(void) {
  uint8_t temp;
  char cBuf[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  String id = "";
  
  W25Q_CHIP_CS_LOW;
  _w25q_spi->transfer(W25Q_CMD_READ_ID);
  _w25q_spi->transfer(0xFF);
  _w25q_spi->transfer(0xFF);
  _w25q_spi->transfer(0xFF);
  _w25q_spi->transfer(0xFF);
  for(uint8_t i = 0; i < 8; i++){
      temp = _w25q_spi->transfer(0xFF);
      //DBG(temp,HEX);
      id += cBuf[(temp>>4) & 0x0F];
      id += cBuf[temp & 0x0F];
  }
  W25Q_CHIP_CS_HIGH;
  return id;
}

uint16_t W25Q::readManufacterAndDeviceID(void) {
  uint16_t val;
  W25Q_CHIP_CS_LOW;
  _w25q_spi->transfer(W25Q_CMD_READ_MDEVICEID);
  _w25q_spi->transfer(0x00);
  _w25q_spi->transfer(0x00);
  _w25q_spi->transfer(0x00);
  val = _w25q_spi->transfer(0xFF) << 8;
  val |= _w25q_spi->transfer(0xFF);
  W25Q_CHIP_CS_HIGH;
  return val;
}