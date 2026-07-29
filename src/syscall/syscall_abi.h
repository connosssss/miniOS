#pragma once
#include <stdint.h>


// Will be shared between kernel and shell
constexpr uint32_t SYS_WRITE = 1;
constexpr uint32_t SYS_GETCHAR = 2;
constexpr uint32_t SYS_READ_FILE = 3;
constexpr uint32_t SYS_LIST_FILES = 4;
constexpr uint32_t SYS_CLEAR = 5;
constexpr uint32_t SYS_CREATE_FILE = 6;
