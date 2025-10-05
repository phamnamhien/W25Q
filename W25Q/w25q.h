/**
 * \file            w25q.h
 * \brief           W25Q flash memory driver
 */

/*
 * Copyright (c) 2025 Pham Nam Hien
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of W25Q flash library.
 *
 * Author:          Pham Nam Hien <phamnamhien@gmail.com>
 * Version:         v1.0.0
 */
#ifndef W25Q_HDR_H
#define W25Q_HDR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           W25Q chip types enumeration
 */
typedef enum {
    W25Q_UNKNOWN = 0x00,                        /*!< Unknown chip */
    W25Q10 = 0x11,                              /*!< W25Q10 - 1Mbit (128KB) */
    W25Q20 = 0x12,                              /*!< W25Q20 - 2Mbit (256KB) */
    W25Q40 = 0x13,                              /*!< W25Q40 - 4Mbit (512KB) */
    W25Q80 = 0x14,                              /*!< W25Q80 - 8Mbit (1MB) */
    W25Q16 = 0x15,                              /*!< W25Q16 - 16Mbit (2MB) */
    W25Q32 = 0x16,                              /*!< W25Q32 - 32Mbit (4MB) */
    W25Q64 = 0x17,                              /*!< W25Q64 - 64Mbit (8MB) */
    W25Q128 = 0x18,                             /*!< W25Q128 - 128Mbit (16MB) */
    W25Q256 = 0x19,                             /*!< W25Q256 - 256Mbit (32MB) */
} w25q_type_t;

/**
 * \brief           W25Q result enumeration
 */
typedef enum {
    W25Q_OK = 0x00,                             /*!< Operation successful */
    W25Q_ERR,                                   /*!< General error */
    W25Q_ERR_TIMEOUT,                           /*!< Timeout error */
    W25Q_ERR_PARAM,                             /*!< Invalid parameter */
    W25Q_ERR_BUSY,                              /*!< Device is busy */
} w25q_result_t;

/**
 * \brief           W25Q chip information structure
 */
typedef struct {
    w25q_type_t type;                           /*!< Chip type */
    uint8_t manufacturer_id;                    /*!< Manufacturer ID (0xEF for Winbond) */
    uint8_t device_id;                          /*!< Device ID (capacity) */
    uint32_t capacity_bytes;                    /*!< Total capacity in bytes */
    uint32_t page_size;                         /*!< Page size (256 bytes) */
    uint32_t sector_size;                       /*!< Sector size (4KB) */
    uint32_t block_size;                        /*!< Block size (64KB) */
    uint32_t page_count;                        /*!< Total number of pages */
    uint32_t sector_count;                      /*!< Total number of sectors */
    uint32_t block_count;                       /*!< Total number of blocks */
} w25q_info_t;

/**
 * \brief           Low-level SPI functions structure for portability
 */
typedef struct {
    uint8_t (*init)(void);                      /*!< Initialize SPI peripheral */
    uint8_t (*select)(void);                    /*!< Select chip (CS low) */
    uint8_t (*deselect)(void);                  /*!< Deselect chip (CS high) */
    uint8_t (*transmit)(const uint8_t* data, uint32_t len);  /*!< Transmit data */
    uint8_t (*receive)(uint8_t* data, uint32_t len);         /*!< Receive data */
    uint8_t (*transmit_receive)(const uint8_t* tx_data, uint8_t* rx_data, uint32_t len);  /*!< Full-duplex transfer */
    void (*delay_ms)(uint32_t ms);              /*!< Delay in milliseconds */
} w25q_ll_t;

/**
 * \brief           W25Q device handle structure
 */
typedef struct {
    w25q_info_t info;                           /*!< Chip information */
    w25q_ll_t ll;                               /*!< Low-level functions */
    uint8_t initialized;                        /*!< Initialization flag */
} w25q_t;

/* Public function prototypes */
w25q_result_t   w25q_init(w25q_t* dev, const w25q_ll_t* ll_funcs);
w25q_result_t   w25q_deinit(w25q_t* dev);
w25q_result_t   w25q_read_id(w25q_t* dev, uint8_t* manufacturer_id, uint8_t* device_id);
w25q_result_t   w25q_detect(w25q_t* dev);
w25q_result_t   w25q_read(w25q_t* dev, uint32_t address, uint8_t* data, uint32_t len);
w25q_result_t   w25q_write_page(w25q_t* dev, uint32_t address, const uint8_t* data, uint32_t len);
w25q_result_t   w25q_erase_sector(w25q_t* dev, uint32_t address);
w25q_result_t   w25q_erase_block_32k(w25q_t* dev, uint32_t address);
w25q_result_t   w25q_erase_block_64k(w25q_t* dev, uint32_t address);
w25q_result_t   w25q_erase_chip(w25q_t* dev);
w25q_result_t   w25q_power_down(w25q_t* dev);
w25q_result_t   w25q_wake_up(w25q_t* dev);
w25q_result_t   w25q_get_info(w25q_t* dev, w25q_info_t* info);
uint8_t         w25q_is_busy(w25q_t* dev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* W25Q_HDR_H */

