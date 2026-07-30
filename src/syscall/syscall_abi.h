#pragma once
#include <stdint.h>


// Will be shared between kernel and shell
constexpr uint32_t SYS_WRITE = 1;
constexpr uint32_t SYS_GETCHAR = 2;
constexpr uint32_t SYS_READ_FILE = 3;
constexpr uint32_t SYS_LIST_FILES = 4;
constexpr uint32_t SYS_CLEAR = 5;
constexpr uint32_t SYS_CREATE_FILE = 6;
constexpr uint32_t SYS_POLLCHAR = 7;
constexpr uint32_t SYS_SLEEP = 8;
constexpr uint32_t SYS_SET_COLOR = 9;
constexpr uint32_t SYS_DELETE_FILE = 10;
constexpr uint32_t SYS_YIELD = 11;
constexpr uint32_t SYS_GET_TICKS = 12;
constexpr uint32_t SYS_GET_PROC_COUNT = 13;
constexpr uint32_t SYS_WRITE_AT = 14;
constexpr uint32_t SYS_WRITE_SERIAL = 15;
constexpr uint32_t SYS_LIST_PROCS = 16;


