/**
  ******************************************************************************
  * @file           : my_fdcan.h
  * @brief          : fdcan
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
#ifndef G4XX_BOOTLOADER_MY_FDCAN_H
#define G4XX_BOOTLOADER_MY_FDCAN_H
/*  includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "fdcan.h"
/* USER CODE END Includes */

/*  types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/* USER CODE END ET */

/*  constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
/* USER CODE END EC */

/*  macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define FIFO0_Watermark 2
extern uint8_t upgrade;
extern uint8_t flash_move_start;
extern uint8_t ota_move_end;
extern uint8_t g_sucdata[64];
/* USER CODE END EM */

/* functions  ---------------------------------------------*/
/* USER CODE BEGIN EFP */
void CAN_bsp_init();
uint8_t CAN_Send_Msg(uint32_t id, uint8_t *msg, uint32_t len);
/* USER CODE END EFP */
#endif //G4XX_BOOTLOADER_MY_FDCAN_H
