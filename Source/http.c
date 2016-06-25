#include "http.h"
#include "html.h"
#include "esp8266.h"
#include "string.h"
#include "stdlib.h"
#include "control.h"

#define HTTP_DEVICE_INIT 0
#define HTTP_WIFI_INIT 1

#define WaitReady(_data, _offset) WaitStr("ready\r\n", 7, _data, _offset)

typedef struct{
    u16 id;
    u16 len;
    u8* data;
}HTTPHead_Type;

#define HEAD_200 "HTTP/1.1 200 OK\r\n\r\n"
#define HEAD_204 "HTTP/1.1 204 No Content\r\n\r\n"
#define HEAD_404 "HTTP/1.1 404 Not Found\r\n\r\n"
HTTPHead_Type HeadList[] = {
    {200, sizeof(HEAD_200)-1, (u8*)HEAD_200},
    {204, sizeof(HEAD_204)-1, (u8*)HEAD_204},
    {404, sizeof(HEAD_404)-1, (u8*)HEAD_404},
};


static u8 HttpStatus = HTTP_DEVICE_INIT;

static u8 ReadyFlag = 0;

/**
* @brief  HTTP 初始化
* @param  None
* @retval None
*/
void HTTP_Init(void)
{
    ESP8266_Init();
}

HTTPHead_Type* HTTP_FindHead(u16 id)
{
    u8 i;
    
    for(i=0; i<sizeof(HeadList)/sizeof(HeadList[0]); i++)
    {
        if(id == HeadList[i].id)
        {
            return &HeadList[i];
        }
    }

    return NULL;
}

bool HTTP_SendPage(u8 con_id, u16 head_id, const u8* data, u16 len)
{
    HTTPHead_Type *head = HTTP_FindHead(head_id);
    
    if(!head) return false;
    
    
    if(ESP8266_SendConDataBegin(con_id, len + head->len))
    {
        ESP8266_SendData(head->data, head->len);
        ESP8266_SendData(data, len);
        ESP8266_SendConDataEnd();
        return true;
    }
    return false;
}

/**
* @brief  处理一行 HTTP 数据
* @param  str : 数据指针
* @param  len : 数据长度
* @retval None
*/
bool HTTP_LineHandler(u8 con_id, char* str, u16 len)
{
    char* p_path, *p_cmd;
    char* p_http = strstr(str, " HTTP/1.1");
    char* name;
    u16 path_len, cmd_len;
    
    if(len > 13 && memcmp(str, "GET ", 4) == 0 
        && p_http
        && str[4] == '/')
    {
        p_path = str + 5;
        *p_http = 0;
        
        // 查找是否带参数
        p_cmd = strchr(p_path, '?');
        if(p_cmd)
        {
            path_len = p_cmd - p_path;
            *p_cmd = 0;
            p_cmd += 1;
            
            cmd_len = p_http - p_cmd;
        }
        else
        {
            path_len = p_http - p_path;
        }
        
        // 参数处理
        if(p_cmd && cmd_len > 0)
        {
            ToggleLight();
        }
        
        HAL_Delay(100);
        
        // 路径处理
        if(path_len == 0)
        {
            p_path = "index.html";
            path_len = 10;
        }
        
        HtmlData_Type *page = HTML_Find(p_path, path_len);
        if(page)
        {
            HTTP_SendPage(con_id, 200, page->data, page->len);
        }
        else
        {
            HtmlData_Type *page = HTML_Find("404.html", 8);
            HTTP_SendPage(con_id, 404, page->data, page->len);
        }

        HAL_Delay(100);
        ESP8266_CloseLink(con_id);
    }        
    return false;
}

/**
* @brief  HTTP 任务，分析 HTTP 数据
* @param  None
* @retval None
*/
void HTTP_Task(void)
{
    CommData_Type data;
    u8 *tmp;
    u16 i, len;
    bool line_flag = false;
    
    if(ESP8266_GetData(&data))
    {
        if(memcmp(data.pBuf, "GET ", 4) == 0)
        {
            //data.pBuf[data.len-1] = 0;
            tmp = data.pBuf;
            for(i=0; i<data.len; i++)
            {
                if(!line_flag && (data.pBuf[i] == '\r' || data.pBuf[i] == '\n'))
                {
                    line_flag = true;
                    data.pBuf[i] = 0;
                    len = &data.pBuf[i] - tmp;
                    if(!HTTP_LineHandler(data.conID, (char*)tmp, len))
                    {
                        break;
                    }
                }
                else if(line_flag && data.pBuf[i] != '\r' && data.pBuf[i] != '\n')
                {
                    line_flag = false;
                    tmp = &data.pBuf[i];
                }
            }
        }
        free(data.pBuf);
    }
}
