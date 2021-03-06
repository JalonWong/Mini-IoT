#include "html.h"

static const u8 index_html[] = {
    0x48,0x54,0x54,0x50,0x2f,0x31,0x2e,0x31,0x20,0x32,0x30,0x30,0x20,0x4f,0x4b,0x0d,0x0a,0x0d,0x0a,0x3c,
    0x68,0x74,0x6d,0x6c,0x3e,0x0d,0x0a,0x0d,0x0a,0x3c,0x62,0x6f,0x64,0x79,0x3e,0x0d,0x0a,0x0d,0x0a,0x3c,
    0x66,0x6f,0x72,0x6d,0x20,0x6e,0x61,0x6d,0x65,0x3d,0x22,0x69,0x6e,0x70,0x75,0x74,0x22,0x20,0x61,0x63,
    0x74,0x69,0x6f,0x6e,0x3d,0x22,0x2f,0x22,0x20,0x6d,0x65,0x74,0x68,0x6f,0x64,0x3d,0x22,0x67,0x65,0x74,
    0x22,0x3e,0x0d,0x0a,0x4d,0x61,0x6c,0x65,0x3a,0x0d,0x0a,0x3c,0x69,0x6e,0x70,0x75,0x74,0x20,0x74,0x79,
    0x70,0x65,0x3d,0x22,0x72,0x61,0x64,0x69,0x6f,0x22,0x20,0x6e,0x61,0x6d,0x65,0x3d,0x22,0x53,0x65,0x78,
    0x22,0x20,0x76,0x61,0x6c,0x75,0x65,0x3d,0x22,0x4d,0x61,0x6c,0x65,0x22,0x20,0x63,0x68,0x65,0x63,0x6b,
    0x65,0x64,0x3d,0x22,0x63,0x68,0x65,0x63,0x6b,0x65,0x64,0x22,0x3e,0x0d,0x0a,0x3c,0x62,0x72,0x20,0x2f,
    0x3e,0x0d,0x0a,0x46,0x65,0x6d,0x61,0x6c,0x65,0x3a,0x0d,0x0a,0x3c,0x69,0x6e,0x70,0x75,0x74,0x20,0x74,
    0x79,0x70,0x65,0x3d,0x22,0x72,0x61,0x64,0x69,0x6f,0x22,0x20,0x6e,0x61,0x6d,0x65,0x3d,0x22,0x53,0x65,
    0x78,0x22,0x20,0x76,0x61,0x6c,0x75,0x65,0x3d,0x22,0x46,0x65,0x6d,0x61,0x6c,0x65,0x22,0x3e,0x0d,0x0a,
    0x3c,0x62,0x72,0x20,0x2f,0x3e,0x0d,0x0a,0x3c,0x69,0x6e,0x70,0x75,0x74,0x20,0x74,0x79,0x70,0x65,0x20,
    0x3d,0x22,0x73,0x75,0x62,0x6d,0x69,0x74,0x22,0x20,0x76,0x61,0x6c,0x75,0x65,0x20,0x3d,0x22,0x53,0x75,
    0x62,0x6d,0x69,0x74,0x22,0x3e,0x0d,0x0a,0x3c,0x2f,0x66,0x6f,0x72,0x6d,0x3e,0x0d,0x0a,0x0d,0x0a,0x3c,
    0x2f,0x62,0x6f,0x64,0x79,0x3e,0x0d,0x0a,0x3c,0x2f,0x68,0x74,0x6d,0x6c,0x3e,0x0d,0x0a,
};



static const u8 _404_html[] = {
    0x48,0x54,0x54,0x50,0x2f,0x31,0x2e,0x31,0x20,0x34,0x30,0x34,0x20,0x4e,0x6f,0x74,0x20,0x46,0x6f,0x75,
    0x6e,0x64,0x0d,0x0a,0x0d,0x0a,0x3c,0x21,0x44,0x4f,0x43,0x54,0x59,0x50,0x45,0x20,0x68,0x74,0x6d,0x6c,
    0x3e,0x0d,0x0a,0x3c,0x68,0x74,0x6d,0x6c,0x3e,0x0d,0x0a,0x3c,0x68,0x65,0x61,0x64,0x3e,0x0d,0x0a,0x20,
    0x20,0x20,0x20,0x3c,0x74,0x69,0x74,0x6c,0x65,0x3e,0x3c,0x2f,0x74,0x69,0x74,0x6c,0x65,0x3e,0x0d,0x0a,
    0x3c,0x2f,0x68,0x65,0x61,0x64,0x3e,0x0d,0x0a,0x3c,0x62,0x6f,0x64,0x79,0x3e,0x0d,0x0a,0x20,0x20,0x20,
    0x20,0x3c,0x21,0x2d,0x2d,0x20,0xe8,0xb0,0x83,0xe7,0x94,0xa8,0x41,0x6e,0x64,0x72,0x6f,0x69,0x64,0xe4,
    0xbb,0xa3,0xe7,0xa0,0x81,0xe4,0xb8,0xad,0xe7,0x9a,0x84,0xe6,0x96,0xb9,0xe6,0xb3,0x95,0x20,0x2d,0x2d,
    0x3e,0x0d,0x0a,0x20,0x20,0x20,0x20,0x3c,0x61,0x20,0x3e,0x0d,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x34,0x30,0x34,0x20,0x4e,0x6f,0x74,0x20,0x46,0x6f,0x75,0x6e,0x64,0x21,0x0d,0x0a,0x20,0x20,0x20,
    0x20,0x3c,0x2f,0x61,0x3e,0x0d,0x0a,0x3c,0x2f,0x62,0x6f,0x64,0x79,0x3e,0x0d,0x0a,0x3c,0x2f,0x68,0x74,
    0x6d,0x6c,0x3e,0x0d,0x0a,
};


HtmlData_Type HtmlData[] = {
    {"index.html", index_html, sizeof(index_html)},
    {"404.html", _404_html, sizeof(_404_html)},
};
