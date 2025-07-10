/**
  ******************************************************************************
  * @file           : my_bootloader.c
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

/*  includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../Inc/my_bootloader.h"
/* USER CODE END Includes */

/*  typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef void (*Jump_Fun)(void);
/* USER CODE END PTD */

/*  define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/*  function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static uint32_t Read_Start_Mode(void);
/* USER CODE END PFP */

/*  user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern uint8_t ReturnPackets_Ok[5];
extern uint32_t g_devid;
/* USER CODE END 0 */

/**
 * @brief       读取模式
 * @param       **
 * @retval      **
 */
static uint32_t Read_Start_Mode(void)
{
    uint32_t mode = 0;
    flash_read(FLASH_UPGRADE_ADDR, &mode, 1);
    return mode;
}

/**
 * @brief       jumptoapp
 * @param       **
 * @retval      **
 */
void JumpToApp(void)
{
    uint32_t i=0;
    uint32_t App_Addr = FLASH_APP_ADDR;
    Jump_Fun AppJump;;         /* 声明一个函数指针 */

    if (((*(__IO unsigned int *)App_Addr) & 0x2FFC0000) == 0x20000000) //检查栈顶地址是否合法
    {
        HAL_DeInit();

        __disable_irq();

        /* 关闭全局中断 */
        __set_PRIMASK(1);

        /* 设置所有时钟到默认状态，使用HSI时钟 */
        HAL_RCC_DeInit();

        /* 关闭滴答定时器，复位到默认值 */
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL = 0;

        /* 关闭所有中断，清除所有中断挂起标志 */
        for (i = 0; i < 8; i++)
        {
            NVIC->ICER[i]=0xFFFFFFFF;
            NVIC->ICPR[i]=0xFFFFFFFF;
        }

        /* 使能全局中断 */
        __set_PRIMASK(0);

        /* 设置主堆栈指针 */
        __set_MSP(*(uint32_t *)App_Addr);

        /* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
        __set_CONTROL(0);

        /* 跳转到应用程序，首地址是MSP，地址+4是复位中断服务程序地址 */
        AppJump = (Jump_Fun) * (__IO unsigned int *)(App_Addr + 4);

        /* 跳转到系统BootLoader */
        AppJump();

        /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
        while (1)
        {

        }
    }

}


/**
 * @brief       bootloader启动
 * @param       **
 * @retval      **
 */
void Bootloader_Excute(void)
{
    uint32_t app_state = 0;
    app_state = Read_Start_Mode();
    switch (app_state) {
        case STARTUP_FIRST:
            /* 清除app区flash */
            if (falsh_erase_Page(FLASH_APP_ADDR))
            {
                while(1);
            }

            /* 判断是否接收到升级标志 */
            while(!upgrade);

            /* 升级固件 */
            Data_transfer();
            break;
        case STARTUP_NORMOL:
            JumpToApp();
            break;
        case STARTUP_OTANOW:
            /* 清除app区flash */
            if (falsh_erase_Page(FLASH_APP_ADDR))
            {
                while(1);
            }

            /* 判断是否接收到升级标志 */
            while(!upgrade);

            /* 升级固件 */
            Data_transfer();
            break;
        case STARTUP_RESET:
            break;
        default:
            break;
    }
}

/**
 * @brief       canfd->app1数据转移
 * @param       **
 * @retval      **
 */
void Data_transfer(void)
{
    /* 获取app起始地址 */
    uint32_t flash_write_addr = FLASH_APP_ADDR;

    /* 设置一个自动重装定时器 */
    bsp_StartAutoTimer(SOFT_TIME1,50);

    while(1)
    {
        /* 等待主机发送允许写入flash */
        while(!flash_move_start)
        {
            /* 判断定时器是否超时 */
            if(bsp_CheckTimer(SOFT_TIME1))
            {
                HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port,LED_GREEN_Pin);
            }

            /* 判断升级是否完成 */
            if (ota_move_end)
            {
                bsp_StopTimer(SOFT_TIME1);
                JumpToApp();
                break;
            }
        };

        /* 清除写入标志 */
        flash_move_start = 0;

        /* 获取有效数据 */
        uint64_t t_flash_wdata[4] = {0};
        for (int i = 0; i < 4; i++) {
            t_flash_wdata[i] = 0;
            for (int j = 0; j < 8; j++) {
                t_flash_wdata[i] |= (uint64_t)g_sucdata[11 + i * 8 + j] << (j * 8);
            }
        }

        /* 写入flash */
        flash_NoErase_Write(flash_write_addr,t_flash_wdata,8);

        /* 地址递增 */
        flash_write_addr += 32;

        HAL_Delay(5);

        CAN_Send_Msg(g_devid,ReturnPackets_Ok,FDCAN_DLC_BYTES_5);

    }
}

uint16_t CRC16_Verification(uint8_t *ptr, uint16_t len)
{
    uint16_t wcrc = 0xFFFF;   // 预置16位crc寄存器，初值全部为1
    uint16_t temp;            // 定义中间变量
    int i, j;                 // 定义计数
    for (i = 0; i < len; i++) // 循环计算每个数据
    {
        temp = *ptr & 0X00FF;   // 将八位数据与crc寄存器亦或
        ptr++;                  // 指针地址增加，指向下个数据
        wcrc ^= temp;           // 将数据存入crc寄存器
        for (j = 0; j < 8; j++) // 循环计算数据的
        {
            if (wcrc & 0X0001) // 判断右移出的是不是1，如果是1则与多项式进行异或。
            {
                wcrc >>= 1;     // 先将数据右移一位
                wcrc ^= 0XA001; // 与上面的多项式进行异或
            }
            else
            {
                wcrc >>= 1; // 直接移出
            }
        }
    }
    temp = wcrc; // crc的值
    return temp;
}


