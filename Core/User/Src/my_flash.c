/**
  ******************************************************************************
  * @file           : my_flash.c
  * @brief          : flash_read flash_write falsh_base
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



/*  includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../Inc/my_flash.h"
#include "core_cm4.h"
/* USER CODE END Includes */

/*  typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/*  define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/*  function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/*  user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
 * @brief       从指定地址读取一个字 (32位数据)
 * @param       faddr : 读取地址 (此地址必须为4倍数!!)
 * @retval      读取到的数据 (32位)
 */
uint32_t flash_read_word(uint32_t faddr)
{
    return *(volatile uint32_t *)faddr;
}

/**
 * @brief       从指定地址开始读出指定长度的数据
 * @param       raddr : 起始地址
 * @param       pbuf  : 数据指针
 * @param       length: 要读取的字(32位)数,即4个字节的整数倍
 * @retval      无
 */
void flash_read(uint32_t raddr, uint32_t *pbuf, uint32_t length)
{
    uint32_t i;
    for (i = 0; i < length; i++)
    {
        pbuf[i] = flash_read_word(raddr); /* 读取4个字节 */
        raddr += 4;                             /* 偏移4个字节 */
    }
}

/**
 * @brief       获取某个地址所在的flash页.stm32g431cbt6只有512K，即256页
 * @param       addr   : flash地址
 * @retval      0~127 ,addr所在的bank1页编号
 *              128~255,addr所在的bank2页编号+64,需要减去64,才得bank2页编号
 */
uint8_t flash_get_flash_sector(uint32_t addr)
{
    for (int i = 0; i < 64; ++i) {
        if (addr >= BANK1_FLASH_PAGE(i) && addr < BANK1_FLASH_PAGE(i+1))
            return i;
    }
    for (int i = 0; i < 64; ++i) {
        if (addr >= BANK2_FLASH_PAGE(i) && addr < BANK2_FLASH_PAGE(i+1))
            return i + 64;
    }
}

/**
 * @brief       从指定地址开始写入指定长度的数据
 * @param       waddr  : 起始地址(此地址必须为4的倍数!!)
 * @param       pbuf   : 数据指针
 * @param       length : 字(32位)数(就是要写入的32位数据的个数)
 * @retval      无
 */
void flash_write(uint32_t waddr, uint32_t *pbuf, uint32_t length)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
    HAL_StatusTypeDef FlashStatus = HAL_OK;
    uint32_t SectorError = 0;
    uint32_t addrx = 0;
    uint32_t endaddr = 0;

    if (waddr < STM32_FLASH_BASE || waddr % 32 ||      /* 写入地址小于 STM32_FLASH_BASE, 或不是32的整数倍, 非法. */
        waddr > (STM32_FLASH_BASE + STM32_FLASH_SIZE)) /* 写入地址大于 STM32_FLASH_BASE + STM32_FLASH_SIZE, 非法. */
    {
        return;
    }

    addrx = waddr;                /* 写入的起始地址 */
    endaddr = waddr + length * 4; /* 写入的结束地址 */

    HAL_FLASH_Unlock();           /* 解锁 */
    if (addrx < 0X1FF00000)
    {
        while (addrx < endaddr) /* 扫清一切障碍.(对非FFFFFFFF的地方,先擦除) */
        {
            if (flash_read_word(addrx) != 0XFFFFFFFF) /* 有非0XFFFFFFFF的地方,要擦除这个页 */
            {
                FlashEraseInit.Banks = FLASH_BANK_1;
                FlashEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;                /* 擦除类型，页擦除 */
                FlashEraseInit.Page = flash_get_flash_sector(addrx);    /* 要擦除的页 */
                FlashEraseInit.NbPages = 1;                                     /* 一次只擦除一页 */
                if (HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
                {
                    break; /* 发生错误了 */
                }
            }
            else
            {
                addrx += 4;
            }
            FLASH_WaitForLastOperation(FLASH_WAITETIME); /* 等待上次操作完成 */
        }
    }
    FlashStatus = FLASH_WaitForLastOperation(FLASH_WAITETIME); /* 等待上次操作完成 */
    if (FlashStatus == HAL_OK)
    {
        while (waddr < endaddr) /* 写数据 */
        {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, waddr, (uint64_t)*pbuf) != HAL_OK) /* 写入数据 */
            {
                break; /* 写入异常 */
            }
            waddr += 32;
            pbuf += 8;
        }
    }
    HAL_FLASH_Lock(); /* 上锁 */
}


/**
 * @brief       在FLASH 指定位置, 写入指定长度的数据(不擦除写入)
 *   @note      对于将FACTORY区搬运到APP区时，因事先已经将APP区擦除了，所以不需要再次擦除，直接写入即可。
 * @param       waddr   : 起始地址 (此地址必须为4的倍数!!,否则写入出错!)
 * @param       pbuf    : 数据指针
 * @param       length  : 要写入的 字(32位)数(就是要写入的32位数据的个数)
 * @retval      无
 */
void flash_NoErase_Write(uint32_t waddr, uint64_t *pbuf, uint32_t length)
{
    HAL_StatusTypeDef FlashStatus = HAL_OK;

    uint32_t endaddr = 0;

    HAL_FLASH_Unlock(); /* 解锁 */

    endaddr = waddr + length * 4; /* 写入的结束地址 */

    FlashStatus = FLASH_WaitForLastOperation(FLASH_WAITETIME); /* 等待上次操作完成 */

    if (FlashStatus == HAL_OK)
    {
        while (waddr < endaddr) /* 写数据 */
        {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, waddr, *pbuf) != HAL_OK) /* 写入数据 */
            {
                while (1)
                {
                    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port,LED_GREEN_Pin);
                    HAL_Delay(1000);
                }
                break; /* 写入异常 */
            }
            waddr += 8;
            pbuf ++;
        }
    }

    HAL_FLASH_Lock(); /* 上锁 */
}

/**
 * @brief       擦除app_flash
 * @param       waddr : 起始地址
 * @param       **
 * @retval      0 成功  1 失败
 */
uint8_t falsh_erase_Page(uint32_t waddr)
{
    uint32_t FirstPage = 0, NbOfPage = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SECTORError = 0;

    /* 解锁 */
    HAL_FLASH_Unlock();

    /* 获取此地址所在的扇区 */
    FirstPage = flash_get_flash_sector(waddr);

    /* 擦除的页 */
    NbOfPage = 64 - FirstPage;

    EraseInitStruct.Banks = FLASH_BANK_1;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;              /* 擦除类型，页擦除 */
    EraseInitStruct.Page = FirstPage;                               /* 要擦除的页 */
    EraseInitStruct.NbPages = NbOfPage;                             /* 一次只擦除一页 */
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return 1;  /* 失败 */
    }
    FLASH_WaitForLastOperation(FLASH_WAITETIME);


    /* 上锁 */
    HAL_FLASH_Lock();

    return 0;   /* 成功 */
}
/******************************************************************************************/
/* 测试用代码 */
/**
 * @brief       测试写数据(写1个字)
 * @param       waddr : 起始地址
 * @param       wdata : 要写入的数据
 * @retval      读取到的数据
 */
void test_write(uint32_t waddr, uint32_t wdata)
{
    flash_write(waddr, &wdata, 1); /* 写入一个字 */
}
