#pragma once

#include "stdint.h"

enum SYSCALL_FUNC {
    SYS_GET_PID,
    SYS_WRITE,
    SYS_READ,
    SYS_READ_CHAR,
    SYS_WRITE_IN_POS,
    SYS_CALL_2A
};

uint32_t get_pid();

uint32_t write(char *s, uint32_t attr);

void read(char *s);

char read_char();

void write_in_pos(char *s, uint32_t attr, uint32_t pos);

void call_2a();