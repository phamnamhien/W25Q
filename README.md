# W25Q Flash Memory Library

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C Standard](https://img.shields.io/badge/C-C11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))

A portable, hardware-agnostic C library for Winbond W25Q series SPI flash memory chips. Supports auto-detection and works with any microcontroller.

## Features

- Auto-detection of W25Q chip models (W25Q10 through W25Q256)
- Hardware-agnostic design - easy porting to any MCU
- Support for 3-byte and 4-byte addressing (W25Q256)
- Complete API: read, write, erase (sector/block/chip)
- Power management (power-down/wake-up)
- Strict C11 coding standards with full Doxygen documentation
- MISRA-C compliant design

## Supported Chips

| Model | Capacity | JEDEC ID | Sectors | Blocks | Status |
|-------|----------|----------|---------|--------|--------|
| W25Q10 | 128KB | 0xEF4011 | 32 | 2 | ✅ Tested |
| W25Q20 | 256KB | 0xEF4012 | 64 | 4 | ✅ Tested |
| W25Q40 | 512KB | 0xEF4013 | 128 | 8 | ✅ Tested |
| W25Q80 | 1MB | 0xEF4014 | 256 | 16 | ✅ Tested |
| W25Q16 | 2MB | 0xEF4015 | 512 | 32 | ✅ Tested |
| W25Q32 | 4MB | 0xEF4016 | 1024 | 64 | ✅ Tested |
| W25Q64 | 8MB | 0xEF4017 | 2048 | 128 | ✅ Tested |
| W25Q128 | 16MB | 0xEF4018 | 4096 | 256 | ✅ Tested |
| W25Q256 | 32MB | 0xEF4019 | 8192 | 512 | ✅ Tested |

## Quick Start

### 1. Add to Your Project

```bash
git clone https://github.com/phamnamhien/W25Q.git
```

Copy `w25q.h` and `w25q.c` to your project.

### 2. Implement Low-Level Functions

Create a porting layer for your MCU:

```c
#include "w25q.h"

static uint8_t spi_init(void) {
    // Initialize your SPI peripheral
    return 1;
}

static uint8_t spi_select(void) {
    // Pull CS pin LOW
    return 1;
}

static uint8_t spi_deselect(void) {
    // Pull CS pin HIGH
    return 1;
}

static uint8_t spi_transmit(const uint8_t* data, uint32_t len) {
    // Transmit data via SPI
    return 1;
}

static uint8_t spi_receive(uint8_t* data, uint32_t len) {
    // Receive data via SPI
    return 1;
}

static uint8_t spi_transmit_receive(const uint8_t* tx, uint8_t* rx, uint32_t len) {
    // Full-duplex SPI transfer
    return 1;
}

static void delay_ms(uint32_t ms) {
    // Delay in milliseconds
}

static const w25q_ll_t w25q_ll = {
    .init = spi_init,
    .select = spi_select,
    .deselect = spi_deselect,
    .transmit = spi_transmit,
    .receive = spi_receive,
    .transmit_receive = spi_transmit_receive,
    .delay_ms = delay_ms,
};
```

### 3. Use the Library

```c
w25q_t flash;
w25q_result_t result;

// Initialize and auto-detect chip
result = w25q_init(&flash, &w25q_ll);
if (result != W25Q_OK) {
    // Handle error
}

// Get chip information
w25q_info_t info;
w25q_get_info(&flash, &info);
printf("Detected: W25Q%d, %lu KB\n", 
       info.device_id, info.capacity_bytes / 1024);

// Erase sector
w25q_erase_sector(&flash, 0x001000);

// Write data
uint8_t data[256] = {0};
w25q_write_page(&flash, 0x001000, data, 256);

// Read data
w25q_read(&flash, 0x001000, data, 256);

// Power down to save energy
w25q_power_down(&flash);
```

## API Reference

### Initialization

```c
w25q_result_t w25q_init(w25q_t* dev, const w25q_ll_t* ll_funcs);
w25q_result_t w25q_deinit(w25q_t* dev);
w25q_result_t w25q_detect(w25q_t* dev);
w25q_result_t w25q_get_info(w25q_t* dev, w25q_info_t* info);
```

### Read/Write

```c
w25q_result_t w25q_read(w25q_t* dev, uint32_t address, uint8_t* data, uint32_t len);
w25q_result_t w25q_write_page(w25q_t* dev, uint32_t address, const uint8_t* data, uint32_t len);
```

**Note:** 
- Maximum write size: 256 bytes per page
- Must erase before writing
- Data wraps around if write crosses page boundary

### Erase

```c
w25q_result_t w25q_erase_sector(w25q_t* dev, uint32_t address);    // 4KB, ~45ms
w25q_result_t w25q_erase_block_32k(w25q_t* dev, uint32_t address); // 32KB, ~120ms
w25q_result_t w25q_erase_block_64k(w25q_t* dev, uint32_t address); // 64KB, ~150ms
w25q_result_t w25q_erase_chip(w25q_t* dev);                        // Full chip, 10-40s
```

### Utilities

```c
w25q_result_t w25q_read_id(w25q_t* dev, uint8_t* mfr_id, uint8_t* dev_id);
uint8_t       w25q_is_busy(w25q_t* dev);
w25q_result_t w25q_power_down(w25q_t* dev);
w25q_result_t w25q_wake_up(w25q_t* dev);
```

## Platform Examples

### STM32 HAL

```c
static uint8_t spi_transmit(const uint8_t* data, uint32_t len) {
    return (HAL_SPI_Transmit(&hspi1, (uint8_t*)data, len, HAL_MAX_DELAY) == HAL_OK) ? 1 : 0;
}

static uint8_t spi_select(void) {
    HAL_GPIO_WritePin(W25Q_CS_GPIO_Port, W25Q_CS_Pin, GPIO_PIN_RESET);
    return 1;
}
```

### ESP32

```c
static spi_device_handle_t spi;

static uint8_t spi_transmit(const uint8_t* data, uint32_t len) {
    spi_transaction_t trans = {
        .length = len * 8,
        .tx_buffer = data,
    };
    return (spi_device_polling_transmit(spi, &trans) == ESP_OK) ? 1 : 0;
}
```

### Arduino

```c
static uint8_t spi_transmit(const uint8_t* data, uint32_t len) {
    SPI.transfer((void*)data, len);
    return 1;
}

static uint8_t spi_select(void) {
    digitalWrite(CS_PIN, LOW);
    return 1;
}
```

## Memory Organization

```
Page:     256 bytes  (0x000000, 0x000100, 0x000200, ...)
Sector:   4KB        (0x000000, 0x001000, 0x002000, ...)
Block32:  32KB       (0x000000, 0x008000, 0x010000, ...)
Block64:  64KB       (0x000000, 0x010000, 0x020000, ...)
```

## Important Notes

### Write Restrictions

1. **Must erase before write** - Flash can only change bits from 1 to 0
2. **Page size limit** - Maximum 256 bytes per write operation
3. **Page boundary** - Writing across page boundaries causes wrap-around
4. **Alignment** - Best practice to align writes to page boundaries

### Performance

| Operation | Typical Time |
|-----------|-------------|
| Page Program | 0.7ms |
| Sector Erase (4KB) | 45ms |
| Block Erase (32KB) | 120ms |
| Block Erase (64KB) | 150ms |
| Chip Erase | 10-40 seconds |

### Endurance

- Program/Erase Cycles: 100,000 typical
- Data Retention: 20 years
- Implement wear leveling for extended life

## Hardware Setup

### SPI Configuration

- Mode: 0 (CPOL=0, CPHA=0) or 3 (CPOL=1, CPHA=1)
- Max Clock: 104MHz (133MHz for some models)
- Bit Order: MSB first
- Data Size: 8 bits

### Typical Connections

```
MCU          W25Q
-----------  ------
SPI_CLK  --> CLK
SPI_MISO <-- DO (IO1)
SPI_MOSI --> DI (IO0)
GPIO_CS  --> CS
3.3V     --> VCC
GND      --> GND
```

**Note:** W25Q operates at 2.7V-3.6V. Use level shifters if MCU is 5V.

## Troubleshooting

### Chip Not Detected

- Verify SPI connections (CLKW25Q/
├── w25q.h              # Public API header
├── w25q.c              # Implementation
├── examples/
│   ├── stm32/          # STM32 HAL example
│   ├── esp32/          # ESP-IDF example
│   └── arduino/        # Arduino example
├── docs/               # Documentation
├── LICENSE             # MIT License
└── README.md           # This file
```

## Building

No special build requirements. Simply include `w25q.h` and compile `w25q.c` with your project.

**Compiler flags:**
```bash
-std=c11 -Wall -Wextra
```

## Contributing

Contributions are welcome! Please:

1. Follow the existing code style
2. Add tests for new features
3. Update documentation
4. Submit pull requests to `develop` branch

## Coding Standards

- C11 standard
- 4 spaces indentation (no tabs)
- Doxygen documentation for all public APIs
- MISRA-C guidelines where applicable

## Testing

Tested on:
- STM32F1/F4/F7/H7 series
- ESP32/ESP32-S3
- Arduino AVR/ARM platforms
- Raspberry Pi Pico

## License

MIT License - see [LICENSE](LICENSE) file for details.

## References

- [W25Q Datasheet](https://www.winbond.com/hq/search/?keyword=W25Q)
- [SPI Flash Memory Guide](https://www.embedded.com/introduction-to-spi-interface/)

## 💖 Support This Project
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/Q5Q1JW4XS)
[![PayPal](https://img.shields.io/badge/PayPal-00457C?style=for-the-badge&logo=paypal&logoColor=white)](https://paypal.me/phamnamhien)

---

If this library helped your project, consider giving it a ⭐ on GitHub!
