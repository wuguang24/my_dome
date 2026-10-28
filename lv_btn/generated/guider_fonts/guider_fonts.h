/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: MIT
 */
#ifndef GUIDER_FONTS_H
#define GUIDER_FONTS_H
#ifdef __cplusplus
extern "C" {
#endif

#if LVGL_VERSION_MAJOR == 7
#include "lv_font/lv_font.h"
#else
#include "font/lv_font.h"
#endif

LV_FONT_DECLARE(lv_font_simsun_16)
LV_FONT_DECLARE(lv_font_simsun_12)


#ifdef __cplusplus
}
#endif
#endif
