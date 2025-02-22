/**
  ******************************************************************************
  * @file           : my_flash.h
  * @brief          : flash
  ******************************************************************************
  * @version        : V1.0
  * @date           : January 9, 2025
  * @note           The modification history table should be filled with real and
  *                 meaningful data.
  ******************************************************************************
  * Version | Date       | Author | Modification Description
  * ------- | ---------- | ------- | -------------------------------
  * V1.0    | 09-Jan-2025 | liuyanyan | Initial version of the file.
  *
  ******************************************************************************
  */

#ifndef G4XX_BOOTLOADER_MY_FLASH_H
#define G4XX_BOOTLOADER_MY_FLASH_H
/*  includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/*  types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/* USER CODE END ET */

/*  constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
/* USER CODE END EC */

/*  macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
/* FLASH起始地址 */
#define STM32_FLASH_SIZE        0X20000                     /* STM32G431CBT6 FLASH 总大小 */
#define STM32_FLASH_BASE        0x08000000                  /* STM32 FLASH 起始地址 */

#define FLASH_WAITETIME         50000                       /* FLASH等待超时时间 */
#define PAGE_SIZE               0x800                       /* 页大小 */

/* FLASH 扇区的起始地址,分2个bank,每个bank 2kb */
/* BANK1 */
#define BANK1_START_ADDR        ((uint32_t)0x08000000)                      /* Bank1页0起始地址 */
#define BANK1_FLASH_PAGE(n)     (BANK1_START_ADDR + ((n) * PAGE_SIZE))      /* 定义Bank1的页面地址 */

/* BNAK2 */
#define BANK2_START_ADDR        ((uint32_t)0x08040000)                      /* Bank2页0起始地址 */
#define BANK2_FLASH_PAGE(n)     (BANK2_START_ADDR + ((n) * PAGE_SIZE))      /* 定义Bank2的页面地址 */
/* USER CODE END EM */

/* functions  ---------------------------------------------*/
/* USER CODE BEGIN EFP */
uint32_t flash_read_word(uint32_t faddr);
void flash_read(uint32_t raddr, uint32_t *pbuf, uint32_t length);
uint8_t flash_get_flash_sector(uint32_t addr);
void flash_write(uint32_t waddr, uint32_t *pbuf, uint32_t length);
void flash_NoErase_Write(uint32_t waddr, uint64_t *pbuf, uint32_t length);
void test_write(uint32_t waddr, uint32_t wdata);
uint8_t falsh_erase_Page(uint32_t waddr);
/* USER CODE END EFP */


#endif //G4XX_BOOTLOADER_MY_FLASH_H
