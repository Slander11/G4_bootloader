/**
  ******************************************************************************
  * @file           : my_bootloader.h
  * @brief          : bootloader -> app2 -> app1
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

#ifndef G4XX_BOOTLOADER_MY_BOOTLOADER_H
#define G4XX_BOOTLOADER_MY_BOOTLOADER_H
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
/* flash中存储的升级标志位 */
#define STARTUP_FIRST   0xFFFFFFFF  /* 初次下载bootloder，此时flash中无数据，读取出来为oxff */
#define STARTUP_NORMOL  0xAAAAAAAA  /* 升级标志，标志含义不需要升级，直接运行APP。正常启动 */
#define STARTUP_OTANOW  0xBBBBBBBB  /* 现在升级 */
#define STARTUP_RESET   0x5555AAAA  /* 恢复出厂 */
/* flash中关于BOOT,APP等的分区以及大小 */
#define FLASH_UPGRADE_ADDR      BANK1_START_ADDR + PAGE_SIZE * 10      /* 升级标志位存放地址(存放在内部FLASH)*/
#define FLASH_BIN_NAME_ADDR     FLASH_UPGRADE_ADDR + 4                 /* 固件名称存放地址(存放在内部FLASH)，存储当前APP名称*/
#define FLASH_DEVICE_ID_ADDR    FLASH_BIN_NAME_ADDR + 4                /* 设备id存放地址(存放在内部FLASH) */
#define FLASH_APP_ADDR          BANK1_FLASH_PAGE(32)                   /* 第一个应用程序APP起始地址(存放在内部FLASH)           \
                                                                        * 保留 0x08000000~0x0800FFFF 的空间为 Bootloader 使用(共64KB) \
                                                                        */
#define FLASH_APP_SIZE 0X38000 /* 第一个应用程序APP所占用的空间大小*/
/* USER CODE END EM */

/* functions  ---------------------------------------------*/
/* USER CODE BEGIN EFP */
void Bootloader_Excute(void);
void Data_transfer(void);
uint16_t CRC16_Verification(uint8_t *ptr, uint16_t len);
void JumpToApp(void);
/* USER CODE END EFP */

#endif //G4XX_BOOTLOADER_MY_BOOTLOADER_H
