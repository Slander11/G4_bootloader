/**
  ******************************************************************************
  * @file           : my_fdcan.c
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

/*  includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../Inc/my_fdcan.h"
/* USER CODE END Includes */

/*  typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
enum {
    no_enough = 0, // 不够包头大小，8字节
    no_len,        // 数据长度不够
    enough,        // 数据无误
    no_begin,      // 不是帧头
    no_end,        // 不是帧尾
    data_error,    // 数据错误
    crc_error,     // crc错误
    id_error,      // id错误
    hand_shake,    // 握手
    over,          // 升级完成
};

typedef union BitToHex
{
    uint8_t bit[2];
    uint16_t hex;
} BitToHex_value;

/* USER CODE END PTD */

/*  define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/*  function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void CAN_filter_init();
static uint8_t GET_FDCAN_DATA(FDCAN_RxHeaderTypeDef *RxHeader,uint8_t *TXmessage);
/* USER CODE END PFP */

/*  user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
FDCAN_TxHeaderTypeDef TXHeader;
FDCAN_RxHeaderTypeDef RXHeader;
/* 握手信息 */
uint8_t upgrade = 0;
/* 接收到新数据 */
uint8_t flash_move_start = 0;
/* 升级完成 */
uint8_t ota_move_end = 0;

/* 握手成功 */
uint8_t hand_suc = 0;
uint8_t g_sucdata[64] = {0};
/* USER CODE END 0 */

/**
 * @brief       canfd初始化
 * @param       **
 * @retval      **
 */
void CAN_bsp_init()
{
    // 设置滤波器
    CAN_filter_init();
    // 开启接收中断
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    HAL_FDCAN_Start(&hfdcan1);
}

/**
 * @brief       canfd滤波设置
 * @param       **
 * @retval      **
 */
static void CAN_filter_init()
{
    HAL_StatusTypeDef HAL_Status;
    FDCAN_FilterTypeDef sFilterConfig;

    /**配置CAN过滤器*/
    sFilterConfig.IdType = FDCAN_STANDARD_ID;             // 标准ID
    sFilterConfig.FilterIndex = 0;                        // 滤波器索引
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // 过滤器0关联到FIFO0
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;         // 掩码过滤
    sFilterConfig.FilterID1 = 0x222;
    sFilterConfig.FilterID2 = 0x7ff;

    HAL_Status = HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig);
    if (HAL_Status != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief       CANFD 发送一组数据
 * @param       id      : 标准ID(11位)
 *              msg     : 数据
 *              len     : 长度
 * @retval      发送状态 0, 成功; 其余, 失败;
 */
uint8_t CAN_Send_Msg(uint32_t id, uint8_t *msg, uint32_t len)
{
    HAL_StatusTypeDef state;

    TXHeader.Identifier = id;
    TXHeader.IdType = FDCAN_STANDARD_ID;
    TXHeader.TxFrameType = FDCAN_DATA_FRAME;
    TXHeader.DataLength = len;
    TXHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
    TXHeader.BitRateSwitch = FDCAN_BRS_ON;
    TXHeader.FDFormat = FDCAN_FD_CAN;
    TXHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TXHeader.MessageMarker = 0x00;

    if ((hfdcan1.Instance->TXFQS & FDCAN_TXFQS_TFQF) != 0U)
    {
        return 0;
    }
    state = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TXHeader, msg);
    if (state != HAL_OK)
        return state; // 发送状态
    return 0;
}

/**
 * @brief       canfd中断接收回调函数
 * @param       hfdcan ： canfd句柄
 * @retval      **
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    uint8_t TXmessage1[64];
    if (hfdcan == &hfdcan1)
    {
        // 新消息处理
        if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
        {
            /* 从RX FIFO0读取数据 */
            HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RXHeader, TXmessage1);

            /* 获取数据 */
            if (enough == GET_FDCAN_DATA(&RXHeader,TXmessage1))
            {
                flash_move_start = 1;
            }

            /* 重新使能RX FIFO0阈值中断 */
            HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
        }
            // fifo满处理
        else if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_FULL) != RESET)
        {
            for (size_t i = 0; i < FIFO0_Watermark; i++)
            {
                HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RXHeader, TXmessage1);
            }
            HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_FULL, 0);
        }
    }
}

/**
 * @brief       接收数据处理函数
 * @param       RxHeader  ：FDCAN Rx header structure definition
 * @retval      TXmessage ：Data
 */
static uint8_t GET_FDCAN_DATA(FDCAN_RxHeaderTypeDef *RxHeader,uint8_t *TXmessage)
{
    /* 解析数据包 */
    uint16_t size = RxHeader->DataLength;
    if(14 == size)  /* DLC==14，数据长度为48 */
    {
        /* 写入 */
        for (int i = 0; i < 48; ++i) {
            g_sucdata[i] = TXmessage[i];
        }

        /* 判断canid是否为本身id */
        uint32_t t_canid = 0x100;
        t_canid = t_canid | g_sucdata[43];
        if (0x195 != t_canid)
        {
            return id_error;
        }

        /* 判断数据头帧 */
        uint8_t t_head[2] = {0};
        t_head[0] = g_sucdata[0];
        t_head[1] = g_sucdata[1];
        if ((t_head[0] != 0x02) || (t_head[1] != 0xfd))
        {
            return no_begin;
        }

        /* 判断数据大小 */
        uint8_t t_size[2] = {0};
        t_size[0] = g_sucdata[2];
        t_size[1] = g_sucdata[3];
        if ((t_size[0] != 0x00) || (t_size[1] != 0x30))
        {
            return no_len;
        }

        /* 判断数据尾帧 */
        uint8_t t_tail[2] = {0};
        t_tail[0] = g_sucdata[46];
        t_tail[1] = g_sucdata[47];
        if ((t_tail[0] != 0x03) || (t_tail[1] != 0xfc))
        {
            return no_end;
        }

        /* 判断crc校验位 */
        uint16_t t1_crc = ((uint16_t)g_sucdata[45] << 8) | g_sucdata[44];
        uint16_t t2_crc = CRC16_Verification(g_sucdata,44);
        if(t1_crc != t2_crc)
        {
            return crc_error;
        }

        /* 获取报文类型 */
        uint8_t t_type = g_sucdata[5];
        if (t_type != 0x03)
        {
            return data_error;
        }

        /* 获取任务序号 */
        uint16_t t_ordinal_num = ((uint16_t) g_sucdata[8] << 8) | g_sucdata[9];
        static uint16_t s_last_ordinal_num = 0;
        if ((t_ordinal_num <= s_last_ordinal_num) && (t_ordinal_num != 0))
        {
            return data_error;
        }
        s_last_ordinal_num = t_ordinal_num;

        /* 判断指令id */
        uint8_t t_id = g_sucdata[10];

        /* 握手帧 */
        if (t_id == 0x01)
        {
            upgrade = 1;
            uint32_t t_canid = 0x100;
            t_canid = t_canid | g_sucdata[43];
            CAN_Send_Msg(t_canid,g_sucdata,FDCAN_DLC_BYTES_48);
            return hand_shake;
        }

        /* 数据帧 */
        else if (0x02 == t_id)
        {
            return enough;
        }

        /* 结束帧 */
        else if(0x03 == t_id)
        {
            ota_move_end = 1;
            return over;
        }
    }
    else
    {
        return no_enough;
    }
    return enough;
}

