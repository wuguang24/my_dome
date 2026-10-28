/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"


void setup_scr_SAR(lv_ui *ui){

	//Write codes SAR
	ui->SAR = lv_obj_create(NULL);
	lv_obj_set_scrollbar_mode(ui->SAR, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_sar_main_main_default
	static lv_style_t style_sar_main_main_default;
	if (style_sar_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_main_main_default);
	else
		lv_style_init(&style_sar_main_main_default);
	lv_style_set_bg_color(&style_sar_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_opa(&style_sar_main_main_default, 0);
	lv_obj_add_style(ui->SAR, &style_sar_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_cont_1
	ui->SAR_cont_1 = lv_obj_create(ui->SAR);
	lv_obj_set_pos(ui->SAR_cont_1, 0, 0);
	lv_obj_set_size(ui->SAR_cont_1, 480, 320);
	lv_obj_set_scrollbar_mode(ui->SAR_cont_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_sar_cont_1_main_main_default
	static lv_style_t style_sar_cont_1_main_main_default;
	if (style_sar_cont_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_cont_1_main_main_default);
	else
		lv_style_init(&style_sar_cont_1_main_main_default);
	lv_style_set_radius(&style_sar_cont_1_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_cont_1_main_main_default, lv_color_make(0xad, 0xe7, 0xf5));
	lv_style_set_bg_grad_color(&style_sar_cont_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_grad_dir(&style_sar_cont_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_cont_1_main_main_default, 255);
	lv_style_set_border_color(&style_sar_cont_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_sar_cont_1_main_main_default, 2);
	lv_style_set_border_opa(&style_sar_cont_1_main_main_default, 255);
	lv_style_set_pad_left(&style_sar_cont_1_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_cont_1_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_cont_1_main_main_default, 0);
	lv_style_set_pad_bottom(&style_sar_cont_1_main_main_default, 0);
	lv_obj_add_style(ui->SAR_cont_1, &style_sar_cont_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_label_1
	ui->SAR_label_1 = lv_label_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_label_1, 190, 7);
	lv_obj_set_size(ui->SAR_label_1, 100, 32);
	lv_obj_set_scrollbar_mode(ui->SAR_label_1, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->SAR_label_1, "SAR Mode");
	lv_label_set_long_mode(ui->SAR_label_1, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_sar_label_1_main_main_default
	static lv_style_t style_sar_label_1_main_main_default;
	if (style_sar_label_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_label_1_main_main_default);
	else
		lv_style_init(&style_sar_label_1_main_main_default);
	lv_style_set_radius(&style_sar_label_1_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_label_1_main_main_default, lv_color_make(0x00, 0xff, 0x40));
	lv_style_set_bg_grad_color(&style_sar_label_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_label_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_label_1_main_main_default, 255);
	lv_style_set_text_color(&style_sar_label_1_main_main_default, lv_color_make(0xff, 0x00, 0x00));
	lv_style_set_text_font(&style_sar_label_1_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_sar_label_1_main_main_default, 2);
	lv_style_set_text_line_space(&style_sar_label_1_main_main_default, 0);
	lv_style_set_text_align(&style_sar_label_1_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_sar_label_1_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_label_1_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_label_1_main_main_default, 8);
	lv_style_set_pad_bottom(&style_sar_label_1_main_main_default, 0);
	lv_obj_add_style(ui->SAR_label_1, &style_sar_label_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_label_2
	ui->SAR_label_2 = lv_label_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_label_2, 12, 88);
	lv_obj_set_size(ui->SAR_label_2, 80, 32);
	lv_obj_set_scrollbar_mode(ui->SAR_label_2, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->SAR_label_2, "Resp");
	lv_label_set_long_mode(ui->SAR_label_2, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_sar_label_2_main_main_default
	static lv_style_t style_sar_label_2_main_main_default;
	if (style_sar_label_2_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_label_2_main_main_default);
	else
		lv_style_init(&style_sar_label_2_main_main_default);
	lv_style_set_radius(&style_sar_label_2_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_label_2_main_main_default, lv_color_make(0xf7, 0x22, 0x22));
	lv_style_set_bg_grad_color(&style_sar_label_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_label_2_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_label_2_main_main_default, 255);
	lv_style_set_text_color(&style_sar_label_2_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_label_2_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_sar_label_2_main_main_default, 2);
	lv_style_set_text_line_space(&style_sar_label_2_main_main_default, 0);
	lv_style_set_text_align(&style_sar_label_2_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_sar_label_2_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_label_2_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_label_2_main_main_default, 8);
	lv_style_set_pad_bottom(&style_sar_label_2_main_main_default, 0);
	lv_obj_add_style(ui->SAR_label_2, &style_sar_label_2_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_label_3
	ui->SAR_label_3 = lv_label_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_label_3, 12, 138);
	lv_obj_set_size(ui->SAR_label_3, 80, 32);
	lv_obj_set_scrollbar_mode(ui->SAR_label_3, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->SAR_label_3, "HR");
	lv_label_set_long_mode(ui->SAR_label_3, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_sar_label_3_main_main_default
	static lv_style_t style_sar_label_3_main_main_default;
	if (style_sar_label_3_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_label_3_main_main_default);
	else
		lv_style_init(&style_sar_label_3_main_main_default);
	lv_style_set_radius(&style_sar_label_3_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_label_3_main_main_default, lv_color_make(0xf7, 0x22, 0x25));
	lv_style_set_bg_grad_color(&style_sar_label_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_label_3_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_label_3_main_main_default, 255);
	lv_style_set_text_color(&style_sar_label_3_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_label_3_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_sar_label_3_main_main_default, 2);
	lv_style_set_text_line_space(&style_sar_label_3_main_main_default, 0);
	lv_style_set_text_align(&style_sar_label_3_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_sar_label_3_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_label_3_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_label_3_main_main_default, 8);
	lv_style_set_pad_bottom(&style_sar_label_3_main_main_default, 0);
	lv_obj_add_style(ui->SAR_label_3, &style_sar_label_3_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_label_4
	ui->SAR_label_4 = lv_label_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_label_4, 12, 184);
	lv_obj_set_size(ui->SAR_label_4, 80, 32);
	lv_obj_set_scrollbar_mode(ui->SAR_label_4, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->SAR_label_4, "Dist");
	lv_label_set_long_mode(ui->SAR_label_4, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_sar_label_4_main_main_default
	static lv_style_t style_sar_label_4_main_main_default;
	if (style_sar_label_4_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_label_4_main_main_default);
	else
		lv_style_init(&style_sar_label_4_main_main_default);
	lv_style_set_radius(&style_sar_label_4_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_label_4_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_sar_label_4_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_label_4_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_label_4_main_main_default, 255);
	lv_style_set_text_color(&style_sar_label_4_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_label_4_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_sar_label_4_main_main_default, 2);
	lv_style_set_text_line_space(&style_sar_label_4_main_main_default, 0);
	lv_style_set_text_align(&style_sar_label_4_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_sar_label_4_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_label_4_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_label_4_main_main_default, 8);
	lv_style_set_pad_bottom(&style_sar_label_4_main_main_default, 0);
	lv_obj_add_style(ui->SAR_label_4, &style_sar_label_4_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_huxi
	ui->SAR_huxi = lv_label_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_huxi, 100, 88);
	lv_obj_set_size(ui->SAR_huxi, 120, 32);
	lv_obj_set_scrollbar_mode(ui->SAR_huxi, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->SAR_huxi, "--");
	lv_label_set_long_mode(ui->SAR_huxi, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_sar_huxi_main_main_default
	static lv_style_t style_sar_huxi_main_main_default;
	if (style_sar_huxi_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_huxi_main_main_default);
	else
		lv_style_init(&style_sar_huxi_main_main_default);
	lv_style_set_radius(&style_sar_huxi_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_huxi_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_sar_huxi_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_huxi_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_huxi_main_main_default, 255);
	lv_style_set_text_color(&style_sar_huxi_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_huxi_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_sar_huxi_main_main_default, 2);
	lv_style_set_text_line_space(&style_sar_huxi_main_main_default, 0);
	lv_style_set_text_align(&style_sar_huxi_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_sar_huxi_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_huxi_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_huxi_main_main_default, 8);
	lv_style_set_pad_bottom(&style_sar_huxi_main_main_default, 0);
	lv_obj_add_style(ui->SAR_huxi, &style_sar_huxi_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_xinlv
	ui->SAR_xinlv = lv_label_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_xinlv, 100, 138);
	lv_obj_set_size(ui->SAR_xinlv, 120, 32);
	lv_obj_set_scrollbar_mode(ui->SAR_xinlv, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->SAR_xinlv, "--");
	lv_label_set_long_mode(ui->SAR_xinlv, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_sar_xinlv_main_main_default
	static lv_style_t style_sar_xinlv_main_main_default;
	if (style_sar_xinlv_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_xinlv_main_main_default);
	else
		lv_style_init(&style_sar_xinlv_main_main_default);
	lv_style_set_radius(&style_sar_xinlv_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_xinlv_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_sar_xinlv_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_xinlv_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_xinlv_main_main_default, 255);
	lv_style_set_text_color(&style_sar_xinlv_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_xinlv_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_sar_xinlv_main_main_default, 2);
	lv_style_set_text_line_space(&style_sar_xinlv_main_main_default, 0);
	lv_style_set_text_align(&style_sar_xinlv_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_sar_xinlv_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_xinlv_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_xinlv_main_main_default, 8);
	lv_style_set_pad_bottom(&style_sar_xinlv_main_main_default, 0);
	lv_obj_add_style(ui->SAR_xinlv, &style_sar_xinlv_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_juli
	ui->SAR_juli = lv_label_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_juli, 100, 184);
	lv_obj_set_size(ui->SAR_juli, 120, 32);
	lv_obj_set_scrollbar_mode(ui->SAR_juli, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->SAR_juli, "--");
	lv_label_set_long_mode(ui->SAR_juli, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_sar_juli_main_main_default
	static lv_style_t style_sar_juli_main_main_default;
	if (style_sar_juli_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_juli_main_main_default);
	else
		lv_style_init(&style_sar_juli_main_main_default);
	lv_style_set_radius(&style_sar_juli_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_juli_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_sar_juli_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_juli_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_juli_main_main_default, 255);
	lv_style_set_text_color(&style_sar_juli_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_juli_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_sar_juli_main_main_default, 2);
	lv_style_set_text_line_space(&style_sar_juli_main_main_default, 0);
	lv_style_set_text_align(&style_sar_juli_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_sar_juli_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_juli_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_juli_main_main_default, 8);
	lv_style_set_pad_bottom(&style_sar_juli_main_main_default, 0);
	lv_obj_add_style(ui->SAR_juli, &style_sar_juli_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_label_5
	ui->SAR_label_5 = lv_label_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_label_5, 275, 71);
	lv_obj_set_size(ui->SAR_label_5, 183, 74);
	lv_obj_set_scrollbar_mode(ui->SAR_label_5, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->SAR_label_5, "Offline mode for no network or GPS failure");
	lv_label_set_long_mode(ui->SAR_label_5, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_sar_label_5_main_main_default
	static lv_style_t style_sar_label_5_main_main_default;
	if (style_sar_label_5_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_label_5_main_main_default);
	else
		lv_style_init(&style_sar_label_5_main_main_default);
	lv_style_set_radius(&style_sar_label_5_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_label_5_main_main_default, lv_color_make(0x07, 0xa9, 0xdf));
	lv_style_set_bg_grad_color(&style_sar_label_5_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_label_5_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_label_5_main_main_default, 255);
	lv_style_set_text_color(&style_sar_label_5_main_main_default, lv_color_make(0xff, 0x00, 0x00));
	lv_style_set_text_font(&style_sar_label_5_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_sar_label_5_main_main_default, 2);
	lv_style_set_text_line_space(&style_sar_label_5_main_main_default, 0);
	lv_style_set_text_align(&style_sar_label_5_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_sar_label_5_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_label_5_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_label_5_main_main_default, 11);
	lv_style_set_pad_bottom(&style_sar_label_5_main_main_default, 0);
	lv_obj_add_style(ui->SAR_label_5, &style_sar_label_5_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_fengmingqi_btn
	ui->SAR_fengmingqi_btn = lv_btn_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_fengmingqi_btn, 21, 252);
	lv_obj_set_size(ui->SAR_fengmingqi_btn, 100, 50);
	lv_obj_set_scrollbar_mode(ui->SAR_fengmingqi_btn, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_sar_fengmingqi_btn_main_main_default
	static lv_style_t style_sar_fengmingqi_btn_main_main_default;
	if (style_sar_fengmingqi_btn_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_fengmingqi_btn_main_main_default);
	else
		lv_style_init(&style_sar_fengmingqi_btn_main_main_default);
	lv_style_set_radius(&style_sar_fengmingqi_btn_main_main_default, 5);
	lv_style_set_bg_color(&style_sar_fengmingqi_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_sar_fengmingqi_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_fengmingqi_btn_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_fengmingqi_btn_main_main_default, 255);
	lv_style_set_border_color(&style_sar_fengmingqi_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_sar_fengmingqi_btn_main_main_default, 0);
	lv_style_set_border_opa(&style_sar_fengmingqi_btn_main_main_default, 255);
	lv_style_set_text_color(&style_sar_fengmingqi_btn_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_fengmingqi_btn_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_align(&style_sar_fengmingqi_btn_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->SAR_fengmingqi_btn, &style_sar_fengmingqi_btn_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->SAR_fengmingqi_btn_label = lv_label_create(ui->SAR_fengmingqi_btn);
	lv_label_set_text(ui->SAR_fengmingqi_btn_label, "Buzzer");
	lv_obj_set_style_pad_all(ui->SAR_fengmingqi_btn, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->SAR_fengmingqi_btn_label, LV_ALIGN_CENTER, 0, 0);

	//Write codes SAR_led_btn
	ui->SAR_led_btn = lv_btn_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_led_btn, 177, 252);
	lv_obj_set_size(ui->SAR_led_btn, 100, 50);
	lv_obj_set_scrollbar_mode(ui->SAR_led_btn, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_sar_led_btn_main_main_default
	static lv_style_t style_sar_led_btn_main_main_default;
	if (style_sar_led_btn_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_led_btn_main_main_default);
	else
		lv_style_init(&style_sar_led_btn_main_main_default);
	lv_style_set_radius(&style_sar_led_btn_main_main_default, 5);
	lv_style_set_bg_color(&style_sar_led_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_sar_led_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_led_btn_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_led_btn_main_main_default, 255);
	lv_style_set_border_color(&style_sar_led_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_sar_led_btn_main_main_default, 0);
	lv_style_set_border_opa(&style_sar_led_btn_main_main_default, 255);
	lv_style_set_text_color(&style_sar_led_btn_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_led_btn_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_align(&style_sar_led_btn_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->SAR_led_btn, &style_sar_led_btn_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->SAR_led_btn_label = lv_label_create(ui->SAR_led_btn);
	lv_label_set_text(ui->SAR_led_btn_label, "LED");
	lv_obj_set_style_pad_all(ui->SAR_led_btn, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->SAR_led_btn_label, LV_ALIGN_CENTER, 0, 0);

	//Write codes SAR_label_6
	ui->SAR_label_6 = lv_label_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_label_6, 281, 154);
	lv_obj_set_size(ui->SAR_label_6, 60, 32);
	lv_obj_set_scrollbar_mode(ui->SAR_label_6, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->SAR_label_6, "Temp");
	lv_label_set_long_mode(ui->SAR_label_6, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_sar_label_6_main_main_default
	static lv_style_t style_sar_label_6_main_main_default;
	if (style_sar_label_6_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_label_6_main_main_default);
	else
		lv_style_init(&style_sar_label_6_main_main_default);
	lv_style_set_radius(&style_sar_label_6_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_label_6_main_main_default, lv_color_make(0xf7, 0x77, 0x22));
	lv_style_set_bg_grad_color(&style_sar_label_6_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_label_6_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_label_6_main_main_default, 255);
	lv_style_set_text_color(&style_sar_label_6_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_label_6_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_sar_label_6_main_main_default, 2);
	lv_style_set_text_line_space(&style_sar_label_6_main_main_default, 0);
	lv_style_set_text_align(&style_sar_label_6_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_sar_label_6_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_label_6_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_label_6_main_main_default, 8);
	lv_style_set_pad_bottom(&style_sar_label_6_main_main_default, 0);
	lv_obj_add_style(ui->SAR_label_6, &style_sar_label_6_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_label_7
	ui->SAR_label_7 = lv_label_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_label_7, 281, 198);
	lv_obj_set_size(ui->SAR_label_7, 60, 32);
	lv_obj_set_scrollbar_mode(ui->SAR_label_7, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->SAR_label_7, "Humi");
	lv_label_set_long_mode(ui->SAR_label_7, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_sar_label_7_main_main_default
	static lv_style_t style_sar_label_7_main_main_default;
	if (style_sar_label_7_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_label_7_main_main_default);
	else
		lv_style_init(&style_sar_label_7_main_main_default);
	lv_style_set_radius(&style_sar_label_7_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_label_7_main_main_default, lv_color_make(0x3d, 0xc8, 0xe1));
	lv_style_set_bg_grad_color(&style_sar_label_7_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_label_7_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_label_7_main_main_default, 255);
	lv_style_set_text_color(&style_sar_label_7_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_label_7_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_sar_label_7_main_main_default, 2);
	lv_style_set_text_line_space(&style_sar_label_7_main_main_default, 0);
	lv_style_set_text_align(&style_sar_label_7_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_sar_label_7_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_label_7_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_label_7_main_main_default, 8);
	lv_style_set_pad_bottom(&style_sar_label_7_main_main_default, 0);
	lv_obj_add_style(ui->SAR_label_7, &style_sar_label_7_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_wendu
	ui->SAR_wendu = lv_label_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_wendu, 346, 154);
	lv_obj_set_size(ui->SAR_wendu, 100, 32);
	lv_obj_set_scrollbar_mode(ui->SAR_wendu, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->SAR_wendu, "--");
	lv_label_set_long_mode(ui->SAR_wendu, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_sar_wendu_main_main_default
	static lv_style_t style_sar_wendu_main_main_default;
	if (style_sar_wendu_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_wendu_main_main_default);
	else
		lv_style_init(&style_sar_wendu_main_main_default);
	lv_style_set_radius(&style_sar_wendu_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_wendu_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_sar_wendu_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_wendu_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_wendu_main_main_default, 255);
	lv_style_set_text_color(&style_sar_wendu_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_wendu_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_sar_wendu_main_main_default, 2);
	lv_style_set_text_line_space(&style_sar_wendu_main_main_default, 0);
	lv_style_set_text_align(&style_sar_wendu_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_sar_wendu_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_wendu_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_wendu_main_main_default, 8);
	lv_style_set_pad_bottom(&style_sar_wendu_main_main_default, 0);
	lv_obj_add_style(ui->SAR_wendu, &style_sar_wendu_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_shidu
	ui->SAR_shidu = lv_label_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_shidu, 346, 198);
	lv_obj_set_size(ui->SAR_shidu, 100, 32);
	lv_obj_set_scrollbar_mode(ui->SAR_shidu, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->SAR_shidu, "--");
	lv_label_set_long_mode(ui->SAR_shidu, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_sar_shidu_main_main_default
	static lv_style_t style_sar_shidu_main_main_default;
	if (style_sar_shidu_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_shidu_main_main_default);
	else
		lv_style_init(&style_sar_shidu_main_main_default);
	lv_style_set_radius(&style_sar_shidu_main_main_default, 0);
	lv_style_set_bg_color(&style_sar_shidu_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_sar_shidu_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_shidu_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_shidu_main_main_default, 255);
	lv_style_set_text_color(&style_sar_shidu_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_shidu_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_sar_shidu_main_main_default, 2);
	lv_style_set_text_line_space(&style_sar_shidu_main_main_default, 0);
	lv_style_set_text_align(&style_sar_shidu_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_sar_shidu_main_main_default, 0);
	lv_style_set_pad_right(&style_sar_shidu_main_main_default, 0);
	lv_style_set_pad_top(&style_sar_shidu_main_main_default, 8);
	lv_style_set_pad_bottom(&style_sar_shidu_main_main_default, 0);
	lv_obj_add_style(ui->SAR_shidu, &style_sar_shidu_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SAR_MAIN_btn
	ui->SAR_MAIN_btn = lv_btn_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_MAIN_btn, 2, 2);
	lv_obj_set_size(ui->SAR_MAIN_btn, 100, 50);
	lv_obj_set_scrollbar_mode(ui->SAR_MAIN_btn, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_sar_main_btn_main_main_default
	static lv_style_t style_sar_main_btn_main_main_default;
	if (style_sar_main_btn_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_main_btn_main_main_default);
	else
		lv_style_init(&style_sar_main_btn_main_main_default);
	lv_style_set_radius(&style_sar_main_btn_main_main_default, 5);
	lv_style_set_bg_color(&style_sar_main_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_sar_main_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_main_btn_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_main_btn_main_main_default, 255);
	lv_style_set_border_color(&style_sar_main_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_sar_main_btn_main_main_default, 0);
	lv_style_set_border_opa(&style_sar_main_btn_main_main_default, 255);
	lv_style_set_text_color(&style_sar_main_btn_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_main_btn_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_align(&style_sar_main_btn_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->SAR_MAIN_btn, &style_sar_main_btn_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->SAR_MAIN_btn_label = lv_label_create(ui->SAR_MAIN_btn);
	lv_label_set_text(ui->SAR_MAIN_btn_label, "Back");
	lv_obj_set_style_pad_all(ui->SAR_MAIN_btn, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->SAR_MAIN_btn_label, LV_ALIGN_CENTER, 0, 0);

	//Write codes SAR_Fault_Diagnosis_btn
	ui->SAR_Fault_Diagnosis_btn = lv_btn_create(ui->SAR_cont_1);
	lv_obj_set_pos(ui->SAR_Fault_Diagnosis_btn, 375, 2);
	lv_obj_set_size(ui->SAR_Fault_Diagnosis_btn, 100, 50);
	lv_obj_set_scrollbar_mode(ui->SAR_Fault_Diagnosis_btn, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_sar_fault_diagnosis_btn_main_main_default
	static lv_style_t style_sar_fault_diagnosis_btn_main_main_default;
	if (style_sar_fault_diagnosis_btn_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_sar_fault_diagnosis_btn_main_main_default);
	else
		lv_style_init(&style_sar_fault_diagnosis_btn_main_main_default);
	lv_style_set_radius(&style_sar_fault_diagnosis_btn_main_main_default, 5);
	lv_style_set_bg_color(&style_sar_fault_diagnosis_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_sar_fault_diagnosis_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_sar_fault_diagnosis_btn_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_sar_fault_diagnosis_btn_main_main_default, 255);
	lv_style_set_border_color(&style_sar_fault_diagnosis_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_sar_fault_diagnosis_btn_main_main_default, 0);
	lv_style_set_border_opa(&style_sar_fault_diagnosis_btn_main_main_default, 255);
	lv_style_set_text_color(&style_sar_fault_diagnosis_btn_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_sar_fault_diagnosis_btn_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_align(&style_sar_fault_diagnosis_btn_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->SAR_Fault_Diagnosis_btn, &style_sar_fault_diagnosis_btn_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->SAR_Fault_Diagnosis_btn_label = lv_label_create(ui->SAR_Fault_Diagnosis_btn);
	lv_label_set_text(ui->SAR_Fault_Diagnosis_btn_label, "Fault");
	lv_obj_set_style_pad_all(ui->SAR_Fault_Diagnosis_btn, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->SAR_Fault_Diagnosis_btn_label, LV_ALIGN_CENTER, 0, 0);

	//Init events for screen
	events_init_SAR(ui);
}

