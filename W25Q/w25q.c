/**
 * \file            w25q.c
 * \brief           W25Q flash memory driver implementation
 */

/*
 * Copyright (c) 2025 Your Name
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
 * Author:          Your Name <your.email@example.com>
 * Version:         v1.0.0
 */
#include "w25q.h"
#include <stddef.h>

/* W25Q command definitions */
#define W25Q_CMD_WRITE_ENABLE           0x06
#define W25Q_CMD_WRITE_DISABLE          0x04
#define W25Q_CMD_READ_STATUS_REG1       0x05
#define W25Q_CMD_READ_STATUS_REG2       0x35
#define W25Q_CMD_WRITE_STATUS_REG       0x01
#define W25Q_CMD_PAGE_PROGRAM           0x02
#define W25Q_CMD_QUAD_PAGE_PROGRAM      0x32
#define W25Q_CMD_BLOCK_ERASE_64K        0xD8
#define W25Q_CMD_BLOCK_ERASE_32K        0x52
#define W25Q_CMD_SECTOR_ERASE_4K        0x20
#define W25Q_CMD_CHIP_ERASE             0xC7
#define W25Q_CMD_ERASE_SUSPEND          0x75
#define W25Q_CMD_ERASE_RESUME           0x7A
#define W25Q_CMD_POWER_DOWN             0xB9
#define W25Q_CMD_RELEASE_POWER_DOWN     0xAB
#define W25Q_CMD_DEVICE_ID              0xAB
#define W25Q_CMD_MANUFACTURER_DEVICE_ID 0x90
#define W25Q_CMD_JEDEC_ID               0x9F
#define W25Q_CMD_READ_DATA              0x03
#define W25Q_CMD_FAST_READ              0x0B
#define W25Q_CMD_READ_UNIQUE_ID         0x4B

/* Status register bit definitions */
#define W25Q_STATUS_BUSY                0x01
#define W25Q_STATUS_WEL                 0x02

/* Chip parameters */
#define W25Q_PAGE_SIZE                  256
#define W25Q_SECTOR_SIZE                4096
#define W25Q_BLOCK_SIZE                 65536
#define W25Q_MANUFACTURER_WINBOND       0xEF
#define W25Q_TIMEOUT_MS                 5000

/**
 * \brief           Wait until device is ready (not busy)
 * \param[in]       dev: W25Q device handle
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
static w25q_result_t
prv_wait_ready(w25q_t* dev) {
    uint8_t status;
    uint32_t timeout;

    timeout = W25Q_TIMEOUT_MS;
    do {
        dev->ll.select();
        dev->ll.transmit((const uint8_t[]){W25Q_CMD_READ_STATUS_REG1}, 1);
        dev->ll.receive(&status, 1);
        dev->ll.deselect();

        if ((status & W25Q_STATUS_BUSY) == 0) {
            return W25Q_OK;
        }

        dev->ll.delay_ms(1);
        timeout--;
    } while (timeout > 0);

    return W25Q_ERR_TIMEOUT;
}

/**
 * \brief           Enable write operations
 * \param[in]       dev: W25Q device handle
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
static w25q_result_t
prv_write_enable(w25q_t* dev) {
    dev->ll.select();
    dev->ll.transmit((const uint8_t[]){W25Q_CMD_WRITE_ENABLE}, 1);
    dev->ll.deselect();

    return W25Q_OK;
}

/**
 * \brief           Get chip capacity based on device ID
 * \param[in]       device_id: Device ID from chip
 * \return          Capacity in bytes, `0` if unknown
 */
static uint32_t
prv_get_capacity(uint8_t device_id) {
    switch (device_id) {
        case W25Q10:  return 131072UL;      /* 128KB */
        case W25Q20:  return 262144UL;      /* 256KB */
        case W25Q40:  return 524288UL;      /* 512KB */
        case W25Q80:  return 1048576UL;     /* 1MB */
        case W25Q16:  return 2097152UL;     /* 2MB */
        case W25Q32:  return 4194304UL;     /* 4MB */
        case W25Q64:  return 8388608UL;     /* 8MB */
        case W25Q128: return 16777216UL;    /* 16MB */
        case W25Q256: return 33554432UL;    /* 32MB */
        default:      return 0;
    }
}

/**
 * \brief           Initialize W25Q device
 * \param[in]       dev: W25Q device handle
 * \param[in]       ll_funcs: Low-level function pointers for SPI communication
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_init(w25q_t* dev, const w25q_ll_t* ll_funcs) {
    if (dev == NULL || ll_funcs == NULL) {
        return W25Q_ERR_PARAM;
    }

    /* Copy low-level functions */
    dev->ll = *ll_funcs;

    /* Initialize SPI */
    if (dev->ll.init != NULL) {
        if (dev->ll.init() == 0) {
            return W25Q_ERR;
        }
    }

    /* Deselect chip */
    dev->ll.deselect();

    /* Wake up chip if it was in power-down mode */
    w25q_wake_up(dev);

    /* Detect chip type and configure */
    if (w25q_detect(dev) != W25Q_OK) {
        return W25Q_ERR;
    }

    dev->initialized = 1;
    return W25Q_OK;
}

/**
 * \brief           Deinitialize W25Q device
 * \param[in]       dev: W25Q device handle
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_deinit(w25q_t* dev) {
    if (dev == NULL || dev->initialized == 0) {
        return W25Q_ERR_PARAM;
    }

    dev->initialized = 0;
    return W25Q_OK;
}

/**
 * \brief           Read manufacturer and device ID
 * \param[in]       dev: W25Q device handle
 * \param[out]      manufacturer_id: Pointer to store manufacturer ID
 * \param[out]      device_id: Pointer to store device ID (capacity)
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_read_id(w25q_t* dev, uint8_t* manufacturer_id, uint8_t* device_id) {
    uint8_t jedec_id[3];

    if (dev == NULL || manufacturer_id == NULL || device_id == NULL) {
        return W25Q_ERR_PARAM;
    }

    /* Sử dụng JEDEC ID (0x9F) để đọc đúng capacity ID */
    dev->ll.select();
    dev->ll.transmit((const uint8_t[]){W25Q_CMD_JEDEC_ID}, 1);
    dev->ll.receive(jedec_id, 3);
    dev->ll.deselect();

    *manufacturer_id = jedec_id[0];  /* 0xEF for Winbond */
    *device_id = jedec_id[2];        /* Capacity: 0x15 for W25Q16 */

    return W25Q_OK;
}

/**
 * \brief           Auto-detect chip type and configure device info
 * \param[in]       dev: W25Q device handle
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_detect(w25q_t* dev) {
    uint8_t manufacturer_id, device_id;
    uint32_t capacity;

    if (dev == NULL) {
        return W25Q_ERR_PARAM;
    }

    /* Read chip ID */
    if (w25q_read_id(dev, &manufacturer_id, &device_id) != W25Q_OK) {
        return W25Q_ERR;
    }

    /* Verify manufacturer */
    if (manufacturer_id != W25Q_MANUFACTURER_WINBOND) {
        dev->info.type = W25Q_UNKNOWN;
        return W25Q_ERR;
    }

    /* Get capacity */
    capacity = prv_get_capacity(device_id);
    if (capacity == 0) {
        dev->info.type = W25Q_UNKNOWN;
        return W25Q_ERR;
    }

    /* Configure device info */
    dev->info.type = (w25q_type_t)device_id;
    dev->info.manufacturer_id = manufacturer_id;
    dev->info.device_id = device_id;
    dev->info.capacity_bytes = capacity;
    dev->info.page_size = W25Q_PAGE_SIZE;
    dev->info.sector_size = W25Q_SECTOR_SIZE;
    dev->info.block_size = W25Q_BLOCK_SIZE;
    dev->info.page_count = capacity / W25Q_PAGE_SIZE;
    dev->info.sector_count = capacity / W25Q_SECTOR_SIZE;
    dev->info.block_count = capacity / W25Q_BLOCK_SIZE;

    return W25Q_OK;
}

/**
 * \brief           Read data from flash memory
 * \param[in]       dev: W25Q device handle
 * \param[in]       address: Start address to read from
 * \param[out]      data: Buffer to store read data
 * \param[in]       len: Number of bytes to read
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_read(w25q_t* dev, uint32_t address, uint8_t* data, uint32_t len) {
    uint8_t cmd[4];

    if (dev == NULL || data == NULL || len == 0) {
        return W25Q_ERR_PARAM;
    }

    if (address + len > dev->info.capacity_bytes) {
        return W25Q_ERR_PARAM;
    }

    /* Wait until device is ready */
    if (prv_wait_ready(dev) != W25Q_OK) {
        return W25Q_ERR_TIMEOUT;
    }

    cmd[0] = W25Q_CMD_READ_DATA;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    dev->ll.select();
    dev->ll.transmit(cmd, 4);
    dev->ll.receive(data, len);
    dev->ll.deselect();

    return W25Q_OK;
}

/**
 * \brief           Write data to a page (max 256 bytes)
 * \param[in]       dev: W25Q device handle
 * \param[in]       address: Page-aligned address to write to
 * \param[in]       data: Data to write
 * \param[in]       len: Number of bytes to write (max 256)
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_write_page(w25q_t* dev, uint32_t address, const uint8_t* data, uint32_t len) {
    uint8_t cmd[4];

    if (dev == NULL || data == NULL || len == 0 || len > W25Q_PAGE_SIZE) {
        return W25Q_ERR_PARAM;
    }

    if (address + len > dev->info.capacity_bytes) {
        return W25Q_ERR_PARAM;
    }

    /* Wait until device is ready */
    if (prv_wait_ready(dev) != W25Q_OK) {
        return W25Q_ERR_TIMEOUT;
    }

    /* Enable write */
    prv_write_enable(dev);

    cmd[0] = W25Q_CMD_PAGE_PROGRAM;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    dev->ll.select();
    dev->ll.transmit(cmd, 4);
    dev->ll.transmit(data, len);
    dev->ll.deselect();

    /* Wait for write completion */
    return prv_wait_ready(dev);
}

/**
 * \brief           Erase 4KB sector
 * \param[in]       dev: W25Q device handle
 * \param[in]       address: Sector address (must be sector-aligned)
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_erase_sector(w25q_t* dev, uint32_t address) {
    uint8_t cmd[4];

    if (dev == NULL) {
        return W25Q_ERR_PARAM;
    }

    if (address >= dev->info.capacity_bytes) {
        return W25Q_ERR_PARAM;
    }

    /* Wait until device is ready */
    if (prv_wait_ready(dev) != W25Q_OK) {
        return W25Q_ERR_TIMEOUT;
    }

    /* Enable write */
    prv_write_enable(dev);

    cmd[0] = W25Q_CMD_SECTOR_ERASE_4K;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    dev->ll.select();
    dev->ll.transmit(cmd, 4);
    dev->ll.deselect();

    /* Wait for erase completion */
    return prv_wait_ready(dev);
}

/**
 * \brief           Erase 32KB block
 * \param[in]       dev: W25Q device handle
 * \param[in]       address: Block address (must be 32KB-aligned)
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_erase_block_32k(w25q_t* dev, uint32_t address) {
    uint8_t cmd[4];

    if (dev == NULL) {
        return W25Q_ERR_PARAM;
    }

    if (address >= dev->info.capacity_bytes) {
        return W25Q_ERR_PARAM;
    }

    /* Wait until device is ready */
    if (prv_wait_ready(dev) != W25Q_OK) {
        return W25Q_ERR_TIMEOUT;
    }

    /* Enable write */
    prv_write_enable(dev);

    cmd[0] = W25Q_CMD_BLOCK_ERASE_32K;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    dev->ll.select();
    dev->ll.transmit(cmd, 4);
    dev->ll.deselect();

    /* Wait for erase completion */
    return prv_wait_ready(dev);
}

/**
 * \brief           Erase 64KB block
 * \param[in]       dev: W25Q device handle
 * \param[in]       address: Block address (must be 64KB-aligned)
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_erase_block_64k(w25q_t* dev, uint32_t address) {
    uint8_t cmd[4];

    if (dev == NULL) {
        return W25Q_ERR_PARAM;
    }

    if (address >= dev->info.capacity_bytes) {
        return W25Q_ERR_PARAM;
    }

    /* Wait until device is ready */
    if (prv_wait_ready(dev) != W25Q_OK) {
        return W25Q_ERR_TIMEOUT;
    }

    /* Enable write */
    prv_write_enable(dev);

    cmd[0] = W25Q_CMD_BLOCK_ERASE_64K;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    dev->ll.select();
    dev->ll.transmit(cmd, 4);
    dev->ll.deselect();

    /* Wait for erase completion */
    return prv_wait_ready(dev);
}

/**
 * \brief           Erase entire chip
 * \param[in]       dev: W25Q device handle
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_erase_chip(w25q_t* dev) {
    if (dev == NULL) {
        return W25Q_ERR_PARAM;
    }

    /* Wait until device is ready */
    if (prv_wait_ready(dev) != W25Q_OK) {
        return W25Q_ERR_TIMEOUT;
    }

    /* Enable write */
    prv_write_enable(dev);

    dev->ll.select();
    dev->ll.transmit((const uint8_t[]){W25Q_CMD_CHIP_ERASE}, 1);
    dev->ll.deselect();

    /* Wait for erase completion */
    return prv_wait_ready(dev);
}

/**
 * \brief           Put device into power-down mode
 * \param[in]       dev: W25Q device handle
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_power_down(w25q_t* dev) {
    if (dev == NULL) {
        return W25Q_ERR_PARAM;
    }

    dev->ll.select();
    dev->ll.transmit((const uint8_t[]){W25Q_CMD_POWER_DOWN}, 1);
    dev->ll.deselect();

    return W25Q_OK;
}

/**
 * \brief           Wake up device from power-down mode
 * \param[in]       dev: W25Q device handle
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_wake_up(w25q_t* dev) {
    if (dev == NULL) {
        return W25Q_ERR_PARAM;
    }

    dev->ll.select();
    dev->ll.transmit((const uint8_t[]){W25Q_CMD_RELEASE_POWER_DOWN}, 1);
    dev->ll.deselect();

    /* Wait for device to wake up (typical 3us) */
    dev->ll.delay_ms(1);

    return W25Q_OK;
}

/**
 * \brief           Get chip information
 * \param[in]       dev: W25Q device handle
 * \param[out]      info: Pointer to store chip information
 * \return          \ref W25Q_OK on success, member of \ref w25q_result_t otherwise
 */
w25q_result_t
w25q_get_info(w25q_t* dev, w25q_info_t* info) {
    if (dev == NULL || info == NULL) {
        return W25Q_ERR_PARAM;
    }

    *info = dev->info;
    return W25Q_OK;
}

/**
 * \brief           Check if device is busy
 * \param[in]       dev: W25Q device handle
 * \return          `1` if busy, `0` if ready
 */
uint8_t
w25q_is_busy(w25q_t* dev) {
    uint8_t status;

    if (dev == NULL) {
        return 0;
    }

    dev->ll.select();
    dev->ll.transmit((const uint8_t[]){W25Q_CMD_READ_STATUS_REG1}, 1);
    dev->ll.receive(&status, 1);
    dev->ll.deselect();

    return (status & W25Q_STATUS_BUSY) ? 1 : 0;
}

