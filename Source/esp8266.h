#ifndef _ESP_8266_H__
#define _ESP_8266_H__

#include "stm32_lib.h"

#define RECV_HALF 0x01
#define RECV_CPLT 0x02
#define RECV_IDLE 0x04

typedef struct{
    u8 *pBuf;
    u16 len;
    u16 size;
    u8 conID;
}CommData_Type;

void ESP8266_Init(void);
void ESP8266_Task(void);
void ESP8266_RecvCallback(u8 state);
void ESP8266_Enable(bool e);

bool ESP8266_SetWiFiMode(u8 mode);
bool ESP8266_SetCipMode(bool e);
bool ESP8266_MuxModeEnable(bool e);
bool ESP8266_ServerEnable(bool e);
bool ESP8266_GetIP(u8 *pbuf, u8 size);
bool ESP8266_GetData(CommData_Type *data);
bool ESP8266_SendConData(u8 con_id, const u8 *pbuf, u16 len);
bool ESP8266_CloseLink(u8 con_id);

#endif
