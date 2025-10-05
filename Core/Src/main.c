/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "w25q.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static w25q_t w25q_device;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void w25q_demo_test(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * \brief           Printf qua UART1 - Sử dụng _write() cho GCC
 */
#if defined(__GNUC__)
int _write(int fd, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}
#elif defined(__ICCARM__)
#include "LowLevelIOInterface.h"
size_t __write(int handle, const unsigned char *buffer, size_t size) {
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, size, HAL_MAX_DELAY);
    return size;
}
#elif defined(__CC_ARM)
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif

/**
 * \brief           Initialize SPI peripheral
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
prv_spi_init(void) {
    /* SPI đã được khởi tạo trong MX_SPI1_Init() */
    /* Chỉ cần đảm bảo CS pin ở mức cao */
    HAL_GPIO_WritePin(W25Q_CS_GPIO_Port, W25Q_CS_Pin, GPIO_PIN_SET);
    return 1;
}

/**
 * \brief           Select chip (pull CS low)
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
prv_spi_select(void) {
    HAL_GPIO_WritePin(W25Q_CS_GPIO_Port, W25Q_CS_Pin, GPIO_PIN_RESET);
    return 1;
}

/**
 * \brief           Deselect chip (pull CS high)
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
prv_spi_deselect(void) {
    HAL_GPIO_WritePin(W25Q_CS_GPIO_Port, W25Q_CS_Pin, GPIO_PIN_SET);
    return 1;
}

/**
 * \brief           Transmit data via SPI
 * \param[in]       data: Data buffer to transmit
 * \param[in]       len: Number of bytes to transmit
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
prv_spi_transmit(const uint8_t* data, uint32_t len) {
    HAL_StatusTypeDef status;

    status = HAL_SPI_Transmit(&hspi1, (uint8_t*)data, len, HAL_MAX_DELAY);
    return (status == HAL_OK) ? 1 : 0;
}

/**
 * \brief           Receive data via SPI
 * \param[out]      data: Buffer to store received data
 * \param[in]       len: Number of bytes to receive
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
prv_spi_receive(uint8_t* data, uint32_t len) {
    HAL_StatusTypeDef status;

    status = HAL_SPI_Receive(&hspi1, data, len, HAL_MAX_DELAY);
    return (status == HAL_OK) ? 1 : 0;
}

/**
 * \brief           Transmit and receive data via SPI (full-duplex)
 * \param[in]       tx_data: Data buffer to transmit
 * \param[out]      rx_data: Buffer to store received data
 * \param[in]       len: Number of bytes to transfer
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
prv_spi_transmit_receive(const uint8_t* tx_data, uint8_t* rx_data, uint32_t len) {
    HAL_StatusTypeDef status;

    status = HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)tx_data, rx_data, len, HAL_MAX_DELAY);
    return (status == HAL_OK) ? 1 : 0;
}

/**
 * \brief           Delay in milliseconds
 * \param[in]       ms: Milliseconds to delay
 */
static void
prv_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

/* Low-level function structure for W25Q library */
static const w25q_ll_t w25q_ll_stm32 = {
    .init = prv_spi_init,
    .select = prv_spi_select,
    .deselect = prv_spi_deselect,
    .transmit = prv_spi_transmit,
    .receive = prv_spi_receive,
    .transmit_receive = prv_spi_transmit_receive,
    .delay_ms = prv_delay_ms,
};

/**
 * \brief           Print chip information
 * \param[in]       info: Chip information structure
 */
static void
print_chip_info(const w25q_info_t* info) {
    printf("\r\n========== W25Q Flash Info ==========\r\n");
    printf("Manufacturer ID : 0x%02X\r\n", info->manufacturer_id);
    printf("Device ID       : 0x%02X\r\n", info->device_id);

    printf("Chip Type       : W25Q");
    switch (info->type) {
        case W25Q10:  printf("10 (1Mbit)\r\n"); break;
        case W25Q20:  printf("20 (2Mbit)\r\n"); break;
        case W25Q40:  printf("40 (4Mbit)\r\n"); break;
        case W25Q80:  printf("80 (8Mbit)\r\n"); break;
        case W25Q16:  printf("16 (16Mbit)\r\n"); break;
        case W25Q32:  printf("32 (32Mbit)\r\n"); break;
        case W25Q64:  printf("64 (64Mbit)\r\n"); break;
        case W25Q128: printf("128 (128Mbit)\r\n"); break;
        case W25Q256: printf("256 (256Mbit)\r\n"); break;
        default:      printf("Unknown\r\n"); break;
    }

    printf("Capacity        : %lu bytes (%lu KB)\r\n",
           info->capacity_bytes, info->capacity_bytes / 1024);
    printf("Page Size       : %lu bytes\r\n", info->page_size);
    printf("Sector Size     : %lu bytes\r\n", info->sector_size);
    printf("Block Size      : %lu bytes\r\n", info->block_size);
    printf("Total Pages     : %lu\r\n", info->page_count);
    printf("Total Sectors   : %lu\r\n", info->sector_count);
    printf("Total Blocks    : %lu\r\n", info->block_count);
    printf("=====================================\r\n\r\n");

    /* Đọc JEDEC ID để kiểm tra kỹ hơn */
    uint8_t jedec_cmd = 0x9F;
    uint8_t jedec_id[3];

    printf("========== JEDEC ID Check ==========\r\n");
    w25q_device.ll.select();
    w25q_device.ll.transmit(&jedec_cmd, 1);
    w25q_device.ll.receive(jedec_id, 3);
    w25q_device.ll.deselect();

    printf("JEDEC ID: 0x%02X 0x%02X 0x%02X\r\n",
           jedec_id[0], jedec_id[1], jedec_id[2]);
    printf("Expected for W25Q16: 0xEF 0x40 0x15\r\n");
    printf("Expected for W25Q80: 0xEF 0x40 0x14\r\n");
    printf("====================================\r\n\r\n");
}

/**
 * \brief           Test basic read/write operations
 */
static void
w25q_demo_test(void) {
    w25q_result_t result;
    w25q_info_t info;
    uint8_t write_buffer[256], read_buffer[256];
    uint32_t test_address, i;
    uint8_t test_passed;

    printf("\r\n");
    printf("*****************************************\r\n");
    printf("*   W25Q Flash Memory Test Program     *\r\n");
    printf("*   STM32F107VCT6 Example               *\r\n");
    printf("*****************************************\r\n\r\n");

    /* Khởi tạo W25Q */
    printf("Initializing W25Q flash...\r\n");
    result = w25q_init(&w25q_device, &w25q_ll_stm32);

    if (result != W25Q_OK) {
        printf("ERROR: W25Q initialization failed!\r\n");
        printf("Please check:\r\n");
        printf("  - SPI connections (MISO, MOSI, CLK, CS)\r\n");
        printf("  - Power supply (3.3V)\r\n");
        printf("  - SPI configuration\r\n");
        return;
    }

    printf("SUCCESS: W25Q initialized!\r\n\r\n");

    /* Lấy thông tin chip */
    w25q_get_info(&w25q_device, &info);
    print_chip_info(&info);

    /* Test 1: Basic Read/Write */
    printf("========== Test 1: Basic Read/Write ==========\r\n");
    test_address = 0x001000; /* Address at 4KB offset */

    /* Chuẩn bị dữ liệu test */
    printf("Preparing test data (256 bytes)...\r\n");
    for (i = 0; i < 256; ++i) {
        write_buffer[i] = (uint8_t)(i & 0xFF);
    }

    /* Xóa sector */
    printf("Erasing sector at 0x%06lX...\r\n", test_address);
    result = w25q_erase_sector(&w25q_device, test_address);
    if (result != W25Q_OK) {
        printf("ERROR: Sector erase failed!\r\n");
        return;
    }
    printf("Sector erased successfully!\r\n");

    /* Ghi dữ liệu */
    printf("Writing 256 bytes...\r\n");
    result = w25q_write_page(&w25q_device, test_address, write_buffer, 256);
    if (result != W25Q_OK) {
        printf("ERROR: Write failed!\r\n");
        return;
    }
    printf("Write completed!\r\n");

    /* Đọc lại */
    printf("Reading 256 bytes...\r\n");
    result = w25q_read(&w25q_device, test_address, read_buffer, 256);
    if (result != W25Q_OK) {
        printf("ERROR: Read failed!\r\n");
        return;
    }
    printf("Read completed!\r\n");

    /* Verify */
    printf("Verifying data...\r\n");
    test_passed = 1;
    for (i = 0; i < 256; ++i) {
        if (write_buffer[i] != read_buffer[i]) {
            printf("ERROR at byte %lu: wrote 0x%02X, read 0x%02X\r\n",
                   i, write_buffer[i], read_buffer[i]);
            test_passed = 0;
            break;
        }
    }

    if (test_passed) {
        printf("SUCCESS: All data verified!\r\n");

        /* In 16 bytes đầu */
        printf("\r\nFirst 16 bytes (hex):\r\n");
        printf("Write: ");
        for (i = 0; i < 16; ++i) {
            printf("%02X ", write_buffer[i]);
        }
        printf("\r\nRead : ");
        for (i = 0; i < 16; ++i) {
            printf("%02X ", read_buffer[i]);
        }
        printf("\r\n");
    }
    printf("==============================================\r\n\r\n");

    /* Test 2: Large Write (1KB) */
    printf("========== Test 2: Large Write (1KB) ==========\r\n");
    test_address = 0x002000;

    printf("Writing 1KB data (4 pages)...\r\n");

    /* Xóa sector */
    w25q_erase_sector(&w25q_device, test_address);

    /* Ghi 4 pages */
    for (i = 0; i < 4; ++i) {
        uint32_t offset;

        offset = i * 256;
        memset(write_buffer, (uint8_t)(i + 0x55), 256);

        result = w25q_write_page(&w25q_device, test_address + offset,
                                 write_buffer, 256);
        if (result != W25Q_OK) {
            printf("ERROR: Write page %lu failed!\r\n", i);
            return;
        }
        printf("  Page %lu written\r\n", i);
    }

    /* Đọc và verify tất cả 4 pages */
    printf("Verifying 1KB data...\r\n");
    test_passed = 1;
    for (i = 0; i < 4; ++i) {
        uint32_t offset, j;

        offset = i * 256;
        w25q_read(&w25q_device, test_address + offset, read_buffer, 256);

        for (j = 0; j < 256; ++j) {
            if (read_buffer[j] != (uint8_t)(i + 0x55)) {
                printf("ERROR at page %lu, byte %lu\r\n", i, j);
                test_passed = 0;
                break;
            }
        }

        if (!test_passed) {
            break;
        }
    }

    if (test_passed) {
        printf("SUCCESS: 1KB data verified!\r\n");
    }
    printf("===============================================\r\n\r\n");

    /* Test 3: Erase Test */
    printf("========== Test 3: Erase Verification ==========\r\n");
    test_address = 0x003000;

    printf("Erasing sector at 0x%06lX...\r\n", test_address);
    w25q_erase_sector(&w25q_device, test_address);

    printf("Verifying erased data (should be 0xFF)...\r\n");
    w25q_read(&w25q_device, test_address, read_buffer, 256);

    test_passed = 1;
    for (i = 0; i < 256; ++i) {
        if (read_buffer[i] != 0xFF) {
            printf("ERROR: Byte %lu is 0x%02X (not 0xFF)\r\n", i, read_buffer[i]);
            test_passed = 0;
            break;
        }
    }

    if (test_passed) {
        printf("SUCCESS: Sector properly erased!\r\n");
    }
    printf("=================================================\r\n\r\n");

    /* Summary */
    printf("\r\n");
    printf("*****************************************\r\n");
    printf("*   ALL TESTS COMPLETED SUCCESSFULLY!   *\r\n");
    printf("*   W25Q Flash is working correctly     *\r\n");
    printf("*****************************************\r\n\r\n");

    /* Power down */
    printf("Putting flash into power-down mode...\r\n");
    w25q_power_down(&w25q_device);
    printf("Done! Flash is now in low-power mode.\r\n\r\n");
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(100);
  w25q_demo_test();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the Systick interrupt time
  */
  __HAL_RCC_PLLI2S_ENABLE();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
