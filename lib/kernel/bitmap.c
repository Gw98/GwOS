#include "bitmap.h"

#include "debug.h"
#include "interrupt.h"
#include "print.h"
#include "string.h"

/** 
 * @brief 初始化bitmap
 * 
 */
void bitmap_init(struct bitmap *mp) {
    memset(mp->bits, 0, mp->len);
}

/** 
 * @brief 判断bitmap的第idx位是否为1
 * 
 * @return bool
 */
bool bitmap_check_idx(struct bitmap *mp, uint32_t idx) {
    uint32_t idx_byte = idx / 8;
    uint32_t idx_bit = idx % 8;

    return (((mp->bits[idx_byte]) & (1 << idx_bit)) != 0);
}

/** 
 * @brief 在bitmap中申请连续的cnt个位
 * 
 * @return 成功则返回起始位的下标，否则返回-1
 */
int32_t bitmap_apply_cnt(struct bitmap *mp, uint32_t cnt) {
    for (int i = 0; i < mp->len - cnt; i++) {
        if (bitmap_check_idx(mp, i) == true) continue;

        bool flag = true;
        for (int j = 1; j < cnt; j++) {
            if (bitmap_check_idx(mp, i + j) == true) {
                flag = false;
                break;
            }
        }
        if (flag == true) return i;
    }
    return -1;
}

/**
 * @brief 将bitmap的第idx位的值设置为value
 * 
 */
void bitmap_set_idx(struct bitmap *mp, uint32_t idx, uint8_t value) {
    ASSERT((value == 0) || (value == 1));
    uint32_t idx_byte = idx / 8;
    uint32_t idx_bit = idx % 8;
    if (value == 0) {
        mp->bits[idx_byte] &= ~(1 << idx_bit);
    } else {
        mp->bits[idx_byte] |= (1 << idx_bit);
    }
}
