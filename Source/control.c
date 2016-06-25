#include "control.h"

void ToggleLight(void)
{
    //HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
    HAL_GPIO_TogglePin(CTRL1_GPIO_Port, CTRL1_Pin);
}

void ShakeFilterU8(u8 in_data, u8* out_data, u16 time)
{
    static u8 last_data = 0;
    static u16 i = 0;
    
    bool ret = false;
    
    if(last_data != in_data)
    {
        i = 0;
    }
    else
    {
        if(i > time)
        {
            *out_data = in_data;
        }
        else
        {
            i++;
        }
    }
    
    last_data = in_data;
}

void INT_Task(void)
{
//    static u8 int1_state = 1, last_int1_state = 1;
//    
//    
//    ShakeFilterU8(
//        HAL_GPIO_ReadPin(INT1_GPIO_Port, INT1_Pin),
//        &int1_state,
//        20);
//    
//    if(int1_state != last_int1_state && int1_state)
//    {
//        ToggleLight();
//    }
//    
//    last_int1_state = int1_state;
}
