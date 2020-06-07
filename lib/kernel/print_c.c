#include "print.h"

void put_int(int32_t x, uint32_t b, uint8_t attr){
    // uint8_t s[10];
    // int flag = x > 0 ? 1 : -1;
    // if(x < 0) x = -x;
    // int len = 0;
    // while(x){
    //     if((x % b) >= 0 && (x % b) <= 9) s[len ++] = x % b + '0';
    //     else s[len ++] = x % b + 'A';
    //     x /= b;
    // }
    // if(flag < 0) s[len ++] = '-';
    // s[len] = '\0';
    // uint8_t tmp;
    // for(int i = 0; i < len / 2; i ++){
    //     tmp = s[i];
    //     s[i] = s[len - 1 - i];
    //     s[len - 1 - i] = tmp;
    // }
    // put_str(s, attr);
    if(x < 0){
        put_char('-', attr);
        x = -x;
    }
    put_uint(x, b, attr);
}

void put_uint(uint32_t x, uint32_t b, uint8_t attr){
    uint8_t s[20];
    int len = 0;
    if(x == 0) s[len ++] = '0';
    while(x){
        if((x % b) >= 0 && (x % b) <= 9) s[len ++] = x % b + '0';
        else s[len ++] = x % b - 10 + 'A';
        x /= b;
    }
    s[len] = '\0';

    uint8_t tmp;
    for(int i = 0; i < len / 2; i ++){
        tmp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = tmp;
    }

    put_str(s, attr);
}

void put_char_in_pos(uint8_t c, uint8_t attr, uint8_t pos_x, uint8_t pos_y) {
    put_char_pos(c, attr, pos_x * 80 + pos_y);
}