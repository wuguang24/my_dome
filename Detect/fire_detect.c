#include "fire_detect.h"

// 检查火灾
uint8_t fire_chk(float smk, uint8_t t, int8_t rise)
{
    if (smk > SMK_FIRE) return 1;
    if (rise > RIS_FIRE) return 1;
    if (t > TEMP_FIRE) return 1;
    return 0;
}

// 检查正常
uint8_t nor_chk(float smk, uint8_t t, int8_t rise)
{
    if (smk < SMK_NOR && rise < RIS_NOR && t < TEMP_NOR) 
        return 1;
    return 0;
}

// 检查故障
uint8_t flt_chk(float smk, uint8_t t)
{
    if (smk < 0 || smk > 5000) return 1;
    if (t > 150) return 1;
    return 0;
}

