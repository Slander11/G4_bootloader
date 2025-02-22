/**
  ******************************************************************************
  * @file           : my_time.h
  * @brief          : time
  ******************************************************************************
  * @version        : V1.0
  * @date           : January 9, 2025
  * @note           The modification history table should be filled with real and
  *                 meaningful data.
  ******************************************************************************
  * Version | Date       | Author | Modification Description
  * ------- | ---------- | ------- | -------------------------------
  * V1.0    | 13-Jan-2025 | liuyanyan | Initial version of the file.
  *
  ******************************************************************************
  */

#ifndef G4XX_BOOTLOADER_MY_TIME_H
#define G4XX_BOOTLOADER_MY_TIME_H
/*  includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/*  types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/* 软件定时器枚举 */
typedef enum
{
    SOFT_TIME1 = 0,
    SOFT_TIME2,
    SOFT_TIME3,
    SOFT_TIME4
}SOFT_WARE_TMR;

/* 工作模式枚举 */
typedef enum
{
    TMR_ONCE_MODE = 0,		/* 一次工作模式 */
    TMR_AUTO_MODE = 1		/* 自动定时工作模式 */
}TMR_MODE_E;

/* 定时器结构体，成员变量必须是 volatile, 否则C编译器优化时可能有问题 */
typedef struct
{
    volatile uint8_t Mode;		/* 计数器模式，1次性 */
    volatile uint8_t Flag;		/* 定时到达标志  */
    volatile uint32_t Count;	/* 计数器 */
    volatile uint32_t PreLoad;	/* 计数器预装值 */
}SOFT_TMR;

/* USER CODE END ET */

/*  constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
/* USER CODE END EC */

/*  macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define TIME_DISABLE    0       /* 软件定时器是否启用 0为启用 */
#define TMR_COUNT	    4		/* 软件定时器的个数 （定时器ID范围 0 - 3) */
/* USER CODE END EM */

/* functions  ---------------------------------------------*/
/* USER CODE BEGIN EFP */
/* 提供给其他C文件调用的函数 */
void bsp_InitTimer(void);                               /* 初始化软件定时器变量 */
void bsp_StartTimer(uint8_t _id, uint32_t _period);     /* 单次定时 */
void bsp_StartAutoTimer(uint8_t _id, uint32_t _period); /* 自动定时 */
void bsp_StopTimer(uint8_t _id);                        /* 停止一个定时器 */
uint8_t bsp_CheckTimer(uint8_t _id);                    /* 检测定时器是否超时 */
int32_t bsp_GetRunTime(void);                           /* cpu运行时间 */
/* USER CODE END EFP */


#endif //G4XX_BOOTLOADER_MY_TIME_H
