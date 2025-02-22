/**
  ******************************************************************************
  * @file           : my_time.c
  * @brief          : time
  ******************************************************************************
  * @version        : V1.0
  * @date           : January 13, 2025
  * @note           The modification history table should be filled with real and
  *                 meaningful data.
  ******************************************************************************
  * Version | Date       | Author | Modification Description
  * ------- | ---------- | ------- | -------------------------------
  * V1.0    | 13-Jan-2025 | liuyanyan | Initial version of the file.
  *
  ******************************************************************************
  */

/*  includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../Inc/my_time.h"
/* USER CODE END Includes */

/*  typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/*  define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/*  function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void bsp_SoftTimerDec(SOFT_TMR *_tmr);
/* USER CODE END PFP */

/*  user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* 定于软件定时器结构体变量 */
static SOFT_TMR s_tTmr[TMR_COUNT] = {0};
static __IO uint8_t g_ucEnableSystickISR = 0;	/* 等待变量初始化 */
/*
	全局运行时间，单位1ms
	最长可以表示 24.85天，如果你的产品连续运行时间超过这个数，则必须考虑溢出问题
*/
__IO int32_t g_iRunTime = 0;
__IO int16_t g_IwdgTime = 0;
/* USER CODE END 0 */

/**
 * @brief       初始化软件定时器变量
 * @param       **
 * @retval      **
 */
void bsp_InitTimer(void)
{
    uint8_t i;

    /* 清零所有的软件定时器 */
    for (i = 0; i < TMR_COUNT; i++)
    {
        s_tTmr[i].Count = 0;
        s_tTmr[i].PreLoad = 0;
        s_tTmr[i].Flag = 0;
        s_tTmr[i].Mode = TMR_ONCE_MODE;	/* 缺省是1次性工作模式 */
    }

}

/**
 * @brief       TIM中断回调函数
 * @param       TIM_HandleTypeDef
 * @retval      **
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* USER CODE BEGIN Callback 0 */
    if (htim->Instance == TIM7) {
        HAL_IncTick();
        if(TIME_DISABLE){
            return;
        }
        /* 每隔1ms，对软件定时器的计数器进行减一操作 */
        for (size_t i = 0; i < TMR_COUNT; i++)
        {
            bsp_SoftTimerDec(&s_tTmr[i]);
        }

        /* 全局运行时间每1ms增1 */
        g_iRunTime++;
        if (g_iRunTime == 0x7FFFFFFF)	/* 这个变量是 int32_t 类型，最大数为 0x7FFFFFFF */
        {
            g_iRunTime = 0;
        }

        /* 全局运行时间每1ms增1 */
        g_IwdgTime++;
        if (g_IwdgTime == 1000)     //每隔1000ms喂一次狗
        {
            g_IwdgTime = 0;
            /* 看门狗 */
        }

    }
    /* USER CODE END Callback 0 */

    /* USER CODE BEGIN Callback 1 */

    /* USER CODE END Callback 1 */
}

/**
 * @brief       每隔1ms对所有定时器变量减1。必须被HAL_TIM_PeriodElapsedCallback周期性调用。
 * @param        _tmr : 定时器变量指针
 * @retval      **
 */
static void bsp_SoftTimerDec(SOFT_TMR *_tmr)
{
    if (_tmr->Count > 0)
    {
        /* 如果定时器变量减到1则设置定时器到达标志 */
        if (--_tmr->Count == 0)
        {
            _tmr->Flag = 1;

            /* 如果是自动模式，则自动重装计数器 */
            if(_tmr->Mode == TMR_AUTO_MODE)
            {
                _tmr->Count = _tmr->PreLoad;
            }
        }
    }
}

/**
 * @brief       启动一个定时器，并设置定时周期。
 * @param        _id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*				_period : 定时周期，单位1ms
 * @retval      **
 */
void bsp_StartTimer(uint8_t _id, uint32_t _period)
{
    if (_id >= TMR_COUNT)
    {
        while(1); /* 参数异常，死机等待看门狗复位 */
    }

    __disable_irq();  			/* 关中断 */

    s_tTmr[_id].Count = _period;		/* 实时计数器初值 */
    s_tTmr[_id].PreLoad = _period;		/* 计数器自动重装值，仅自动模式起作用 */
    s_tTmr[_id].Flag = 0;				/* 定时时间到标志 */
    s_tTmr[_id].Mode = TMR_ONCE_MODE;	/* 1次性工作模式 */

    __enable_irq();  			/* 开中断 */
}

/**
 * @brief       启动一个自动定时器，并设置定时周期。
 * @param        _id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*				_period : 定时周期，单位10ms
 * @retval      **
 */
void bsp_StartAutoTimer(uint8_t _id, uint32_t _period)
{
    if (_id >= TMR_COUNT)
    {
        while(1); /* 参数异常，死机等待看门狗复位 */
    }

    __disable_irq();  		/* 关中断 */

    s_tTmr[_id].Count = _period;		/* 实时计数器初值 */
    s_tTmr[_id].PreLoad = _period;		/* 计数器自动重装值，仅自动模式起作用 */
    s_tTmr[_id].Flag = 0;				/* 定时时间到标志 */
    s_tTmr[_id].Mode = TMR_AUTO_MODE;	/* 自动工作模式 */

    __enable_irq();  		/* 开中断 */
}

/**
 * @brief       停止一个定时器
 * @param        _id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
 * @retval      **
 */
void bsp_StopTimer(uint8_t _id)
{
    if (_id >= TMR_COUNT)
    {
        while(1); /* 参数异常，死机等待看门狗复位 */
    }

    __disable_irq();  	/* 关中断 */

    s_tTmr[_id].Count = 0;				/* 实时计数器初值 */
    s_tTmr[_id].Flag = 0;				/* 定时时间到标志 */
    s_tTmr[_id].Mode = TMR_ONCE_MODE;	/* 自动工作模式 */

    __enable_irq();  		/* 开中断 */
}

/**
 * @brief       检测定时器是否超时
 * @param       _id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
 *				_period : 定时周期，单位1ms
 * @retval      返回 0 表示定时未到， 1表示定时到
 */
uint8_t bsp_CheckTimer(uint8_t _id)
{
    if (_id >= TMR_COUNT)
    {
        return 0;
    }

    if (s_tTmr[_id].Flag == 1)
    {
        s_tTmr[_id].Flag = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief       获取CPU运行时间，单位1ms。最长可以表示 24.85天，如果你的产品连续运行时间超过这个数，则必须考虑溢出问题
 * @param       **
 * @retval      CPU运行时间，单位1ms
 */
int32_t bsp_GetRunTime(void)
{
    int32_t runtime;

    __disable_irq();  	    /* 关中断 */

    runtime = g_iRunTime;	/* 这个变量在定时中断中被改写，因此需要关中断进行保护 */

    __enable_irq();  		/* 开中断 */

    return runtime;
}
