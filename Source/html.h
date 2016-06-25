#ifndef __HTML_H__
#define __HTML_H__

#include "stm32_lib.h"

typedef struct {
    char *name;
    const u8 *data;
    u16 len;
}HtmlData_Type;

HtmlData_Type* HTML_Find(const char* name, u16 name_len);

#endif
