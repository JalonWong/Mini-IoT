#include "esp8266.h"
#include "usart.h"
#include "string.h"
#include "stdlib.h"

#define RX_SIZE         128
#define LINE_SIZE       256

#define MODE_CMD        0
#define MODE_GET_ID     1
#define MODE_GET_LEN    2
#define MODE_DATA       3
#define MODE_SEND       4

#define RET_READY       1
#define RET_OK          2
#define RET_ERROR       3
#define RET_NO_CHG      4
#define RET_BUSY        5
#define RET_DATA        6
#define RET_SEND        7
#define RET_SEND_OK     8
#define RET_LINK_NOT    9

typedef struct {
    char *cmd;
    u8 size;
    u8 retFlag;
}CmdData_Type;

CmdData_Type ESP_Cmd[] = {
    {"ready",        5,  RET_READY},
    {"OK",           2,  RET_OK},
    {"ERROR",        5,  RET_ERROR},
    {"no change",    9,  RET_NO_CHG},
    {"busy",         4,  RET_BUSY},
    {"busy now ...", 12, RET_BUSY},
    {"SEND OK",      7,  RET_SEND_OK},
    {"link is not",  11, RET_SEND_OK},
};

//void (*DataCallback)(u8*, u16) = NULL;

static UART_HandleTypeDef *VIEW_USART, *ESP_USART;
//static u8 TaskActive = 0;
static u8 RecvFlag = 0;

static u8 RecvBuffer[RX_SIZE];
static u8 LineBuffer[LINE_SIZE];

static u8 BuffIndex = 0;
static CommData_Type DataBuffer[2] = {
    {NULL, 0, 0, 0},
    {NULL, 0, 0, 0},
};

static u8 LastResponse = 0;
static u8 *RecvLineBuff = NULL;
static u16 RecvLineSize = 0;
static u16 RecvLineLen = 0;

static u8 IP_Data[20];

u8 HandleMode = MODE_CMD;

void ESP8266_SendData(const u8 *pbuf, u16 len);

bool Check(u8 id)
{
    if(LastResponse == id)
    {
        LastResponse = 0;
        return true;
    }
    return false;
}

/**
* @brief  HTTP 初始化
* @param  None
* @retval None
*/
void ESP8266_Init(void)
{
    bool ret;
    
    ESP_USART = &huart2;
    VIEW_USART = &huart1;
    HAL_UART_Receive_DMA(ESP_USART, RecvBuffer, RX_SIZE);
    //HAL_UART_Receive_DMA(ESP_USART, (u8*)&VIEW_USART->Instance->DR, 1);
    HAL_UART_Receive_DMA(VIEW_USART, (u8*)&ESP_USART->Instance->DR, 1);
    
    ESP8266_Enable(true);
    while(!Check(RET_READY)){}
    
    ret = ESP8266_SetWiFiMode(3);
    
    ret = ESP8266_MuxModeEnable(true);
    assert_param(ret);
        
    ret = ESP8266_SetCipMode(false);
    assert_param(ret);
    
    ret = ESP8266_ServerEnable(true, 80);
    assert_param(ret);
    
    //ESP8266_GetIP(IP_Data, sizeof(IP_Data));
}

/**
* @brief  使能 ESP8266 模块
* @param  e : 是否使能
* @retval None
*/
void ESP8266_Enable(bool e)
{
    if(e)
    {
        HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, GPIO_PIN_RESET);
    }
}

/**
* @brief  从数据中分割出一段数据，基于 key 的值
* @param  data: 数据
* @param  pbuf: 缓存并输出数据的 buffer
* @param  size: Buffer Size
* @param  key : 分割数据的键值
* @retval 数据长度
*/
u16 GetSplitData(u8 data, u8 *pbuf, u16 size, u8 key)
{
    static u16 offset = 0;
    u16 ret = 0;
    
    if(offset >= size) 
    {
        offset = 0;
    }
    
    pbuf[offset++] = data;
    
    if(offset >= 2 && pbuf[offset-1] == key)
    {
        ret = offset - 1;
        offset = 0;
    }
    
    return ret;
}

/**
* @brief  从数据中分割出一行数据
* @param  data: 数据
* @param  pbuf: 缓存并输出数据的 buffer
* @param  size: Buffer Size
* @retval 行数据长度
*/
u16 GetLineData(u8 data, u8 *pbuf, u16 size)
{
    static u16 offset = 0;
    u16 ret = 0;
    
    if(data < 1)
    {
        return ret;
    }
    
    if(offset >= size) 
    {
        offset = 0;
    }
    
    pbuf[offset++] = data;
    
    if(pbuf[offset-1] == '\r' || pbuf[offset-1] == '\n')
    {
        if(offset >= 2)
        {
            ret = offset - 1;
        }
        offset = 0;
    }    
    
    // 模式切换
    if(offset == 5 && memcmp(LineBuffer, "+IPD,", 5) == 0)
    {
        ret = 5;
        offset = 0;
        HandleMode = MODE_GET_ID;
    }
    
    return ret;
}

/**
* @brief  逐行处理 Esp 模块返回的数据
* @param  data: 数据
* @retval None
*/
void LineDataHandle(u8 *pbuf, u16 len)
{
    static u8 cmd_size = sizeof(ESP_Cmd)/sizeof(ESP_Cmd[0]);
    u8 i;
    
    // 对比关键字
    for(i=0; i<cmd_size; i++)
    {
        CmdData_Type *p = &ESP_Cmd[i];
        
        if(p->size == len && memcmp(pbuf, p->cmd, len) == 0)
        {
            LastResponse = p->retFlag;
            return;
        }
    }
    
    if(len == 4 && memcmp(pbuf, "Link", 4) == 0)
    {
        return;
    }
    
    if(len == 6 && memcmp(pbuf, "Unlink", 6) == 0)
    {
        return;
    }
    
    // 返回一行数据
    if(RecvLineBuff)
    {
        if(len > RecvLineSize)
        {
            LastResponse = RET_ERROR;
            return;
        }
        
        memcpy(RecvLineBuff, pbuf, len);
        RecvLineLen = len;
        RecvLineBuff = NULL;
        LastResponse = RET_OK;
        return;
    }
}

/**
* @brief  逐个 Byte 的处理 Esp 模块返回的数据
* @param  data: 数据
* @retval None
*/
void DataHandle(u8 data)
{
    static u16 data_index;
    u16 len, data_len;
    CommData_Type *buff;
    
    switch(HandleMode)
    {
    case MODE_CMD:
        len = GetLineData(data, LineBuffer, LINE_SIZE);
        if(len)
        {
            LineDataHandle(LineBuffer, len);
        }
        break;
    case MODE_GET_ID:
        len = GetSplitData(data, LineBuffer, LINE_SIZE, ',');
        if(len)
        {
            DataBuffer[BuffIndex].conID = LineBuffer[0] - '0';
            HandleMode = MODE_GET_LEN;
        }
        break;
    case MODE_GET_LEN:
        len = GetSplitData(data, LineBuffer, LINE_SIZE, ':');
        if(len)
        {
            u8 i;
            
            data_len = 0;
            for(i=0; i<len; i++)
            {
                data_len = data_len * 10 + (LineBuffer[i] - '0');
            }
            data_index = 0;
            
            // 申请和释放内容
            buff = &DataBuffer[BuffIndex];
            buff->len = data_len;
            if(buff->pBuf)
            {
                if(data_len > buff->size)
                {
                    buff->size = data_len;
                    buff->pBuf = realloc(buff->pBuf, buff->size);
                }
            }
            else
            {
                buff->size = data_len;
                buff->pBuf = malloc(buff->size);
            }
            
            HandleMode = MODE_DATA;
        }
        break;
    case MODE_DATA:
        buff = &DataBuffer[BuffIndex];
        if(data_index < buff->len)
        {
            if(buff->pBuf)
            {
                buff->pBuf[data_index] = data;
            }
            data_index++;
        }
        else
        {
            len = GetLineData(data, LineBuffer, LINE_SIZE);
            if(len == 2 && memcmp(LineBuffer, "OK", 2) == 0)
            {
                if(buff->pBuf)
                {
                    BuffIndex ^= 0x01;
                    LastResponse = RET_DATA;
                }
                HandleMode = MODE_CMD;
            }
        }        
        break;
    case MODE_SEND:
        len = GetSplitData(data, LineBuffer, LINE_SIZE, '>');
        if(len > 12 && memcmp(LineBuffer, "AT+CIPSEND=", 11) == 0)
        {
            LastResponse = RET_SEND;
            HandleMode = MODE_CMD;
        }
        break;
    }
}

/**
* @brief  串口 DMA 发送数据
* @param  huart: 串口句柄
* @param  pData: 数据指针
* @param  Size : 数据长度
* @retval 错误码
*/
HAL_StatusTypeDef USER_UART_Transmit_DMA(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size)
{
    u32 tc_flag = __HAL_DMA_GET_TC_FLAG_INDEX(huart->hdmatx);
    
    if(__HAL_DMA_GET_FLAG(huart->hdmatx, tc_flag))
    {
        __HAL_DMA_CLEAR_FLAG(huart->hdmatx, tc_flag);
        __HAL_UNLOCK(huart->hdmatx);
        
        if(huart->State == HAL_UART_STATE_BUSY_TX_RX)
        {
            huart->State = HAL_UART_STATE_BUSY_RX;
        }
        else
        {
            huart->State = HAL_UART_STATE_READY;
        }
    }
    
    return HAL_UART_Transmit_DMA(huart, (u8*)pData, Size);
}

/**
* @brief  向 ESP 模块发送裸数据
* @param  pbuf: 数据指针
* @param  len:  数据长度
* @retval None
*/
void ESP8266_SendData(const u8 *pbuf, u16 len)
{
    if(pbuf == NULL || len == 0) return;
    
    while(USER_UART_Transmit_DMA(ESP_USART, pbuf, len) != HAL_OK) {}
}

void ESP8266_RecvCallback(u8 state)
{  
    RecvFlag |= state;
}

static void SetRecvLineDataBuff(u8 *pbuf, u16 size)
{
    RecvLineBuff = pbuf;
    RecvLineSize = size;
}

// TODO: 超时
bool CheckResponse(u8 ret, u16 timeout)
{
    u16 i = 0;
    while(timeout == 0 || i < timeout)
    {
        if(LastResponse)
        {
            if(Check(ret))
            {
                return true;
            }
            else
            {
                break;
            }
        }
        
//        if(Check(RET_SEND_OK))
//        {
//            return true;
//        }
//        
//        if(Check(RET_OK))
//        {
//            return true;
//        }
//        
//        if(Check(RET_NO_CHG))
//        {
//            return true;
//        }
//        
//        if(Check(RET_ERROR))
//        {
//            return false;
//        }
//        
//        if(Check(RET_BUSY))
//        {
//            return false;
//        }
        
        HAL_Delay(1);
        i++;
    }
    LastResponse = 0;
    return false;
}

bool ESP8266_RecvData(u8 **out_pbuf, u16 *plen)
{
    static u16 offset = 0;
    
    u8 state;
    u8 *pbuf;
    u16 len, data_cnt;
    
    if(RecvFlag)
    {    
        if(RecvFlag & RECV_HALF)
        {
            data_cnt = RX_SIZE / 2;
            state = RECV_HALF;
        }
        else if(RecvFlag & RECV_CPLT)
        {
            data_cnt = RX_SIZE;
            state = RECV_CPLT;
        }
        else if(RecvFlag & RECV_IDLE)
        {
            data_cnt = RX_SIZE - __HAL_DMA_GET_COUNTER(ESP_USART->hdmarx);
            state = RECV_IDLE;
            
            if(data_cnt < offset)
            {
                return false;
            }
        }
        
        pbuf = &RecvBuffer[offset];
        len = data_cnt - offset;
        
        if(USER_UART_Transmit_DMA(VIEW_USART, pbuf, len) == HAL_OK)
        {
            RecvFlag &= ~state;
            offset = data_cnt % RX_SIZE;
            
            *out_pbuf = pbuf;
            *plen = len;
            return true;
        }
    }
    
    return false;
}

void ESP8266_Task(void)
{
    u8 *pbuf;
    u16 len, i;
    
    if(ESP8266_RecvData(&pbuf, &len))
    {
        for(i=0; i<len; i++)
        {
            DataHandle(pbuf[i]);
        }
    }
}

// 接口函数

/**
* @brief  整形数据转字符串
* @param  data : 整形数据
* @param  pbuf : 字符串指针
* @retval 字符串长度
*/
u8 IntToStr(u16 data, u8* str)
{
    u8 i, num;
    u8 num_flag = 0;
    u16 mask = 10000;
    
    for(i=0; i<5 && mask > 0;)
    {
        num = data / mask;
        if(num || num_flag)
        {
            num_flag = 1;
            
            str[i++] = '0' + num;
        }
        
        data = data % mask;
        mask /= 10;
    }
    
    str[i++] = '\r';
    str[i++] = '\n';
    
    return i;
}

/**
* @brief  ESP 模块向指定 socket 链接发送数据
* @param  con_id : 数据指针
* @param  pbuf   : 数据指针
* @param  len    : 数据长度
* @retval 是否成功
*/
//bool ESP8266_SendConData(u8 con_id, const u8 *pbuf, u16 len)
//{
//    static u8 cmd[] = "AT+CIPSEND=0,00000\r\n";
//    u8 i;
//    
//    // 参数检查
//    if(con_id < 10 && pbuf && len > 0)
//    {
//        cmd[11] = '0' + con_id;
//        
//        // 数据长度转字符串
//        i = IntToStr(len, &cmd[13]) + 13;
//        
//        // 发送请求
//        HandleMode = MODE_SEND;
//        LastResponse = 0;
//        ESP8266_SendData(cmd, i);
//        
//        if(!CheckResponse(RET_SEND, 1000))
//        {
//            HandleMode = MODE_CMD;
//            return false;
//        }
//        
//        // 发送数据
//        ESP8266_SendData(pbuf, len);
//        
//        return CheckResponse(RET_SEND_OK, 3000);
//    }
//    return false;
//}

/**
* @brief  ESP 模块开始向指定 socket 链接发送数据
* @param  con_id : 数据指针
* @param  len    : 数据长度
* @retval 开始是否成功
*/
bool ESP8266_SendConDataBegin(u8 con_id, u16 len)
{
    static u8 cmd[] = "AT+CIPSEND=0,00000\r\n";
    u8 i;
    
    // 参数检查
    if(con_id < 10 && len > 0)
    {
        cmd[11] = '0' + con_id;
        
        // 数据长度转字符串
        i = IntToStr(len, &cmd[13]) + 13;
        
        // 发送请求
        HandleMode = MODE_SEND;
        LastResponse = 0;
        ESP8266_SendData(cmd, i);
        
        if(!CheckResponse(RET_SEND, 1000))
        {
            HandleMode = MODE_CMD;
            return false;
        }
        return true;
    }
    return false;
}

/**
* @brief  socket 发送数据结束
* @param  None
* @retval None
*/
bool ESP8266_SendConDataEnd(void)
{
    return CheckResponse(RET_SEND_OK, 3000);
}

bool ESP8266_CloseLink(u8 con_id)
{
    static u8 cmd[] = "AT+CIPCLOSE=0\r\n";
    
    if(con_id < 10)
    {
        cmd[12] = '0' + con_id;
        LastResponse = 0;
        ESP8266_SendData(cmd, sizeof(cmd));
        return CheckResponse(RET_OK, 1000);
    }
    
    return false;
}

bool ESP8266_SetWiFiMode(u8 mode)
{
    static u8 cmd[] = "AT+CWMODE=1\r\n";
    
    if(mode > 0 && mode < 4)
    {
        cmd[10] = '0' + mode;
        
        LastResponse = 0;
        ESP8266_SendData(cmd, sizeof(cmd));
        return CheckResponse(RET_OK, 0);
    }
    return false;
}

/**
* @brief  使能透传模式
* @param  e : 是否使能
* @retval 是否成功
*/
bool ESP8266_SetCipMode(bool e)
{
    static u8 cmd[] = "AT+CIPMODE=0\r\n";
    
    if(e) cmd[11] = '1';
    else cmd[11] = '0';
    
    LastResponse = 0;
    ESP8266_SendData(cmd, sizeof(cmd));
    return CheckResponse(RET_OK, 0);
}

bool ESP8266_MuxModeEnable(bool e)
{
    static u8 cmd[] = "AT+CIPMUX=1\r\n";
    
    if(e) cmd[10] = '1';
    else cmd[10] = '0';
    
    LastResponse = 0;
    ESP8266_SendData(cmd, sizeof(cmd));
    return CheckResponse(RET_OK, 0);
}

bool ESP8266_ServerEnable(bool e, u16 port)
{
    static u8 cmd[] = "AT+CIPSERVER=1,00000\r\n";
    u8 i;
    
    if(e) cmd[13] = '1';
    else cmd[13] = '0';
    
    i = IntToStr(port, &cmd[15]) + 15;
    
    LastResponse = 0;
    ESP8266_SendData(cmd, i);
    return CheckResponse(RET_OK, 0);
}

bool ESP8266_GetIP(u8 *pbuf, u8 size)
{
    static u8 cmd[] = "AT+CIFSR\r\n";
    
    LastResponse = 0;
    ESP8266_SendData(cmd, sizeof(cmd));
    SetRecvLineDataBuff(pbuf, size-1);
    while(1)
    {
        if(Check(RET_OK))
        {
            pbuf[RecvLineLen] = 0;
            return true;
        }
        
        if(Check(RET_ERROR))
        {
            return false;
        }
    }
}

bool ESP8266_GetData(CommData_Type *data)
{
    while(1)
    {
        if(Check(RET_DATA))
        {
            u8 index = BuffIndex^0x01;
            
            if(DataBuffer[index].pBuf)
            {
                *data = DataBuffer[index];
                LastResponse = 0;
            }
            return true;
        }
    }
}
