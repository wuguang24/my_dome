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


void setup_scr_MAIN(lv_ui *ui){

	//Write codes MAIN
	ui->MAIN = lv_obj_create(NULL);
	lv_obj_set_scrollbar_mode(ui->MAIN, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_main_main_main_default
	static lv_style_t style_main_main_main_default;
	if (style_main_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_main_main_default);
	else
		lv_style_init(&style_main_main_main_default);
	lv_style_set_bg_color(&style_main_main_main_default, lv_color_make(0xd8, 0xf3, 0xef));
	lv_style_set_bg_opa(&style_main_main_main_default, 0);
	lv_obj_add_style(ui->MAIN, &style_main_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_cont_1
	ui->MAIN_cont_1 = lv_obj_create(ui->MAIN);
	lv_obj_set_pos(ui->MAIN_cont_1, 0, 0);
	lv_obj_set_size(ui->MAIN_cont_1, 480, 320);
	lv_obj_set_scrollbar_mode(ui->MAIN_cont_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_main_cont_1_main_main_default
	static lv_style_t style_main_cont_1_main_main_default;
	if (style_main_cont_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_cont_1_main_main_default);
	else
		lv_style_init(&style_main_cont_1_main_main_default);
	lv_style_set_radius(&style_main_cont_1_main_main_default, 0);
	lv_style_set_bg_color(&style_main_cont_1_main_main_default, lv_color_make(0x3b, 0x52, 0x78));
	lv_style_set_bg_grad_color(&style_main_cont_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_grad_dir(&style_main_cont_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_cont_1_main_main_default, 255);
	lv_style_set_border_color(&style_main_cont_1_main_main_default, lv_color_make(0x47, 0x6f, 0x90));
	lv_style_set_border_width(&style_main_cont_1_main_main_default, 2);
	lv_style_set_border_opa(&style_main_cont_1_main_main_default, 255);
	lv_style_set_pad_left(&style_main_cont_1_main_main_default, 0);
	lv_style_set_pad_right(&style_main_cont_1_main_main_default, 0);
	lv_style_set_pad_top(&style_main_cont_1_main_main_default, 0);
	lv_style_set_pad_bottom(&style_main_cont_1_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_cont_1, &style_main_cont_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_label15
	ui->MAIN_label15 = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_label15, 20, 6);
	lv_obj_set_size(ui->MAIN_label15, 100, 32);
	lv_obj_set_scrollbar_mode(ui->MAIN_label15, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_label15, "LifeDetect");
	lv_label_set_long_mode(ui->MAIN_label15, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_label15_main_main_default
	static lv_style_t style_main_label15_main_main_default;
	if (style_main_label15_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_label15_main_main_default);
	else
		lv_style_init(&style_main_label15_main_main_default);
	lv_style_set_radius(&style_main_label15_main_main_default, 0);
	lv_style_set_bg_color(&style_main_label15_main_main_default, lv_color_make(0xff, 0x00, 0x00));
	lv_style_set_bg_grad_color(&style_main_label15_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_label15_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_label15_main_main_default, 255);
	lv_style_set_text_color(&style_main_label15_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_label15_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_label15_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_label15_main_main_default, 0);
	lv_style_set_text_align(&style_main_label15_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_label15_main_main_default, 0);
	lv_style_set_pad_right(&style_main_label15_main_main_default, 0);
	lv_style_set_pad_top(&style_main_label15_main_main_default, 8);
	lv_style_set_pad_bottom(&style_main_label15_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_label15, &style_main_label15_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_label_1
	ui->MAIN_label_1 = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_label_1, 0, 104);
	lv_obj_set_size(ui->MAIN_label_1, 60, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_label_1, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_label_1, "Resp");
	lv_label_set_long_mode(ui->MAIN_label_1, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_label_1_main_main_default
	static lv_style_t style_main_label_1_main_main_default;
	if (style_main_label_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_label_1_main_main_default);
	else
		lv_style_init(&style_main_label_1_main_main_default);
	lv_style_set_radius(&style_main_label_1_main_main_default, 0);
	lv_style_set_bg_color(&style_main_label_1_main_main_default, lv_color_make(0xf7, 0x22, 0x25));
	lv_style_set_bg_grad_color(&style_main_label_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_label_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_label_1_main_main_default, 255);
	lv_style_set_text_color(&style_main_label_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_label_1_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_label_1_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_label_1_main_main_default, 0);
	lv_style_set_text_align(&style_main_label_1_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_label_1_main_main_default, 0);
	lv_style_set_pad_right(&style_main_label_1_main_main_default, 0);
	lv_style_set_pad_top(&style_main_label_1_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_label_1_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_label_1, &style_main_label_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_label_2
	ui->MAIN_label_2 = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_label_2, 0, 145);
	lv_obj_set_size(ui->MAIN_label_2, 60, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_label_2, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_label_2, "HR");
	lv_label_set_long_mode(ui->MAIN_label_2, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_label_2_main_main_default
	static lv_style_t style_main_label_2_main_main_default;
	if (style_main_label_2_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_label_2_main_main_default);
	else
		lv_style_init(&style_main_label_2_main_main_default);
	lv_style_set_radius(&style_main_label_2_main_main_default, 0);
	lv_style_set_bg_color(&style_main_label_2_main_main_default, lv_color_make(0xf7, 0x22, 0x25));
	lv_style_set_bg_grad_color(&style_main_label_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_label_2_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_label_2_main_main_default, 255);
	lv_style_set_text_color(&style_main_label_2_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_label_2_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_label_2_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_label_2_main_main_default, 0);
	lv_style_set_text_align(&style_main_label_2_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_label_2_main_main_default, 0);
	lv_style_set_pad_right(&style_main_label_2_main_main_default, 0);
	lv_style_set_pad_top(&style_main_label_2_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_label_2_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_label_2, &style_main_label_2_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_label_3
	ui->MAIN_label_3 = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_label_3, 0, 185);
	lv_obj_set_size(ui->MAIN_label_3, 60, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_label_3, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_label_3, "Motion");
	lv_label_set_long_mode(ui->MAIN_label_3, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_label_3_main_main_default
	static lv_style_t style_main_label_3_main_main_default;
	if (style_main_label_3_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_label_3_main_main_default);
	else
		lv_style_init(&style_main_label_3_main_main_default);
	lv_style_set_radius(&style_main_label_3_main_main_default, 0);
	lv_style_set_bg_color(&style_main_label_3_main_main_default, lv_color_make(0x00, 0xff, 0x88));
	lv_style_set_bg_grad_color(&style_main_label_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_label_3_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_label_3_main_main_default, 255);
	lv_style_set_text_color(&style_main_label_3_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_label_3_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_label_3_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_label_3_main_main_default, 0);
	lv_style_set_text_align(&style_main_label_3_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_label_3_main_main_default, 0);
	lv_style_set_pad_right(&style_main_label_3_main_main_default, 0);
	lv_style_set_pad_top(&style_main_label_3_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_label_3_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_label_3, &style_main_label_3_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_shijian
	ui->MAIN_shijian = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_shijian, 286, 265);
	lv_obj_set_size(ui->MAIN_shijian, 181, 43);
	lv_obj_set_scrollbar_mode(ui->MAIN_shijian, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_shijian, "YYYY-MM-DD HH:MM:SS");
	lv_label_set_long_mode(ui->MAIN_shijian, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_shijian_main_main_default
	static lv_style_t style_main_shijian_main_main_default;
	if (style_main_shijian_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_shijian_main_main_default);
	else
		lv_style_init(&style_main_shijian_main_main_default);
	lv_style_set_radius(&style_main_shijian_main_main_default, 0);
	lv_style_set_bg_color(&style_main_shijian_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_main_shijian_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_shijian_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_shijian_main_main_default, 255);
	lv_style_set_text_color(&style_main_shijian_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_shijian_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_shijian_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_shijian_main_main_default, 0);
	lv_style_set_text_align(&style_main_shijian_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_shijian_main_main_default, 0);
	lv_style_set_pad_right(&style_main_shijian_main_main_default, 0);
	lv_style_set_pad_top(&style_main_shijian_main_main_default, 4);
	lv_style_set_pad_bottom(&style_main_shijian_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_shijian, &style_main_shijian_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_label_4
	ui->MAIN_label_4 = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_label_4, 400, 6);
	lv_obj_set_size(ui->MAIN_label_4, 60, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_label_4, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_label_4, "GPS");
	lv_label_set_long_mode(ui->MAIN_label_4, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_label_4_main_main_default
	static lv_style_t style_main_label_4_main_main_default;
	if (style_main_label_4_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_label_4_main_main_default);
	else
		lv_style_init(&style_main_label_4_main_main_default);
	lv_style_set_radius(&style_main_label_4_main_main_default, 0);
	lv_style_set_bg_color(&style_main_label_4_main_main_default, lv_color_make(0x00, 0x08, 0xff));
	lv_style_set_bg_grad_color(&style_main_label_4_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_label_4_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_label_4_main_main_default, 255);
	lv_style_set_text_color(&style_main_label_4_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_label_4_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_label_4_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_label_4_main_main_default, 0);
	lv_style_set_text_align(&style_main_label_4_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_label_4_main_main_default, 0);
	lv_style_set_pad_right(&style_main_label_4_main_main_default, 0);
	lv_style_set_pad_top(&style_main_label_4_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_label_4_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_label_4, &style_main_label_4_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_label_5
	ui->MAIN_label_5 = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_label_5, 0, 61);
	lv_obj_set_size(ui->MAIN_label_5, 60, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_label_5, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_label_5, "Exist");
	lv_label_set_long_mode(ui->MAIN_label_5, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_label_5_main_main_default
	static lv_style_t style_main_label_5_main_main_default;
	if (style_main_label_5_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_label_5_main_main_default);
	else
		lv_style_init(&style_main_label_5_main_main_default);
	lv_style_set_radius(&style_main_label_5_main_main_default, 0);
	lv_style_set_bg_color(&style_main_label_5_main_main_default, lv_color_make(0xf7, 0x22, 0x25));
	lv_style_set_bg_grad_color(&style_main_label_5_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_label_5_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_label_5_main_main_default, 255);
	lv_style_set_text_color(&style_main_label_5_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_label_5_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_label_5_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_label_5_main_main_default, 0);
	lv_style_set_text_align(&style_main_label_5_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_label_5_main_main_default, 0);
	lv_style_set_pad_right(&style_main_label_5_main_main_default, 0);
	lv_style_set_pad_top(&style_main_label_5_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_label_5_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_label_5, &style_main_label_5_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_label_6
	ui->MAIN_label_6 = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_label_6, 271, 51);
	lv_obj_set_size(ui->MAIN_label_6, 60, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_label_6, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_label_6, "Lon");
	lv_label_set_long_mode(ui->MAIN_label_6, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_label_6_main_main_default
	static lv_style_t style_main_label_6_main_main_default;
	if (style_main_label_6_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_label_6_main_main_default);
	else
		lv_style_init(&style_main_label_6_main_main_default);
	lv_style_set_radius(&style_main_label_6_main_main_default, 0);
	lv_style_set_bg_color(&style_main_label_6_main_main_default, lv_color_make(0x77, 0x22, 0xf7));
	lv_style_set_bg_grad_color(&style_main_label_6_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_label_6_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_label_6_main_main_default, 255);
	lv_style_set_text_color(&style_main_label_6_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_label_6_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_label_6_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_label_6_main_main_default, 0);
	lv_style_set_text_align(&style_main_label_6_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_label_6_main_main_default, 0);
	lv_style_set_pad_right(&style_main_label_6_main_main_default, 0);
	lv_style_set_pad_top(&style_main_label_6_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_label_6_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_label_6, &style_main_label_6_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_label_7
	ui->MAIN_label_7 = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_label_7, 271, 104);
	lv_obj_set_size(ui->MAIN_label_7, 60, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_label_7, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_label_7, "Lat");
	lv_label_set_long_mode(ui->MAIN_label_7, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_label_7_main_main_default
	static lv_style_t style_main_label_7_main_main_default;
	if (style_main_label_7_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_label_7_main_main_default);
	else
		lv_style_init(&style_main_label_7_main_main_default);
	lv_style_set_radius(&style_main_label_7_main_main_default, 0);
	lv_style_set_bg_color(&style_main_label_7_main_main_default, lv_color_make(0x77, 0x22, 0xf7));
	lv_style_set_bg_grad_color(&style_main_label_7_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_label_7_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_label_7_main_main_default, 255);
	lv_style_set_text_color(&style_main_label_7_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_label_7_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_label_7_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_label_7_main_main_default, 0);
	lv_style_set_text_align(&style_main_label_7_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_label_7_main_main_default, 0);
	lv_style_set_pad_right(&style_main_label_7_main_main_default, 0);
	lv_style_set_pad_top(&style_main_label_7_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_label_7_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_label_7, &style_main_label_7_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_label_8
	ui->MAIN_label_8 = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_label_8, 0, 224);
	lv_obj_set_size(ui->MAIN_label_8, 60, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_label_8, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_label_8, "Dist");
	lv_label_set_long_mode(ui->MAIN_label_8, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_label_8_main_main_default
	static lv_style_t style_main_label_8_main_main_default;
	if (style_main_label_8_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_label_8_main_main_default);
	else
		lv_style_init(&style_main_label_8_main_main_default);
	lv_style_set_radius(&style_main_label_8_main_main_default, 0);
	lv_style_set_bg_color(&style_main_label_8_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_main_label_8_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_label_8_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_label_8_main_main_default, 255);
	lv_style_set_text_color(&style_main_label_8_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_label_8_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_label_8_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_label_8_main_main_default, 0);
	lv_style_set_text_align(&style_main_label_8_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_label_8_main_main_default, 0);
	lv_style_set_pad_right(&style_main_label_8_main_main_default, 0);
	lv_style_set_pad_top(&style_main_label_8_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_label_8_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_label_8, &style_main_label_8_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_label_9
	ui->MAIN_label_9 = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_label_9, 271, 162);
	lv_obj_set_size(ui->MAIN_label_9, 60, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_label_9, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_label_9, "Temp");
	lv_label_set_long_mode(ui->MAIN_label_9, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_label_9_main_main_default
	static lv_style_t style_main_label_9_main_main_default;
	if (style_main_label_9_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_label_9_main_main_default);
	else
		lv_style_init(&style_main_label_9_main_main_default);
	lv_style_set_radius(&style_main_label_9_main_main_default, 0);
	lv_style_set_bg_color(&style_main_label_9_main_main_default, lv_color_make(0xa2, 0x91, 0x20));
	lv_style_set_bg_grad_color(&style_main_label_9_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_label_9_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_label_9_main_main_default, 255);
	lv_style_set_text_color(&style_main_label_9_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_label_9_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_label_9_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_label_9_main_main_default, 0);
	lv_style_set_text_align(&style_main_label_9_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_label_9_main_main_default, 0);
	lv_style_set_pad_right(&style_main_label_9_main_main_default, 0);
	lv_style_set_pad_top(&style_main_label_9_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_label_9_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_label_9, &style_main_label_9_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_label_10
	ui->MAIN_label_10 = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_label_10, 272, 204);
	lv_obj_set_size(ui->MAIN_label_10, 60, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_label_10, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_label_10, "Humi");
	lv_label_set_long_mode(ui->MAIN_label_10, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_label_10_main_main_default
	static lv_style_t style_main_label_10_main_main_default;
	if (style_main_label_10_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_label_10_main_main_default);
	else
		lv_style_init(&style_main_label_10_main_main_default);
	lv_style_set_radius(&style_main_label_10_main_main_default, 0);
	lv_style_set_bg_color(&style_main_label_10_main_main_default, lv_color_make(0x01, 0x4b, 0xf9));
	lv_style_set_bg_grad_color(&style_main_label_10_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_label_10_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_label_10_main_main_default, 255);
	lv_style_set_text_color(&style_main_label_10_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_label_10_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_label_10_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_label_10_main_main_default, 0);
	lv_style_set_text_align(&style_main_label_10_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_label_10_main_main_default, 0);
	lv_style_set_pad_right(&style_main_label_10_main_main_default, 0);
	lv_style_set_pad_top(&style_main_label_10_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_label_10_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_label_10, &style_main_label_10_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_cunzi
	ui->MAIN_cunzi = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_cunzi, 61, 61);
	lv_obj_set_size(ui->MAIN_cunzi, 100, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_cunzi, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_cunzi, "--");
	lv_label_set_long_mode(ui->MAIN_cunzi, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_cunzi_main_main_default
	static lv_style_t style_main_cunzi_main_main_default;
	if (style_main_cunzi_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_cunzi_main_main_default);
	else
		lv_style_init(&style_main_cunzi_main_main_default);
	lv_style_set_radius(&style_main_cunzi_main_main_default, 0);
	lv_style_set_bg_color(&style_main_cunzi_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_main_cunzi_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_cunzi_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_cunzi_main_main_default, 255);
	lv_style_set_text_color(&style_main_cunzi_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_cunzi_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_cunzi_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_cunzi_main_main_default, 0);
	lv_style_set_text_align(&style_main_cunzi_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_cunzi_main_main_default, 0);
	lv_style_set_pad_right(&style_main_cunzi_main_main_default, 0);
	lv_style_set_pad_top(&style_main_cunzi_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_cunzi_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_cunzi, &style_main_cunzi_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_huxi
	ui->MAIN_huxi = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_huxi, 61, 104);
	lv_obj_set_size(ui->MAIN_huxi, 100, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_huxi, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_huxi, "--");
	lv_label_set_long_mode(ui->MAIN_huxi, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_huxi_main_main_default
	static lv_style_t style_main_huxi_main_main_default;
	if (style_main_huxi_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_huxi_main_main_default);
	else
		lv_style_init(&style_main_huxi_main_main_default);
	lv_style_set_radius(&style_main_huxi_main_main_default, 0);
	lv_style_set_bg_color(&style_main_huxi_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_main_huxi_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_huxi_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_huxi_main_main_default, 255);
	lv_style_set_text_color(&style_main_huxi_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_huxi_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_huxi_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_huxi_main_main_default, 0);
	lv_style_set_text_align(&style_main_huxi_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_huxi_main_main_default, 0);
	lv_style_set_pad_right(&style_main_huxi_main_main_default, 0);
	lv_style_set_pad_top(&style_main_huxi_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_huxi_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_huxi, &style_main_huxi_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_xinlv
	ui->MAIN_xinlv = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_xinlv, 61, 145);
	lv_obj_set_size(ui->MAIN_xinlv, 100, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_xinlv, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_xinlv, "--");
	lv_label_set_long_mode(ui->MAIN_xinlv, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_xinlv_main_main_default
	static lv_style_t style_main_xinlv_main_main_default;
	if (style_main_xinlv_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_xinlv_main_main_default);
	else
		lv_style_init(&style_main_xinlv_main_main_default);
	lv_style_set_radius(&style_main_xinlv_main_main_default, 0);
	lv_style_set_bg_color(&style_main_xinlv_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_main_xinlv_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_xinlv_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_xinlv_main_main_default, 255);
	lv_style_set_text_color(&style_main_xinlv_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_xinlv_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_xinlv_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_xinlv_main_main_default, 0);
	lv_style_set_text_align(&style_main_xinlv_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_xinlv_main_main_default, 0);
	lv_style_set_pad_right(&style_main_xinlv_main_main_default, 0);
	lv_style_set_pad_top(&style_main_xinlv_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_xinlv_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_xinlv, &style_main_xinlv_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_tidong
	ui->MAIN_tidong = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_tidong, 61, 185);
	lv_obj_set_size(ui->MAIN_tidong, 100, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_tidong, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_tidong, "--");
	lv_label_set_long_mode(ui->MAIN_tidong, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_tidong_main_main_default
	static lv_style_t style_main_tidong_main_main_default;
	if (style_main_tidong_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_tidong_main_main_default);
	else
		lv_style_init(&style_main_tidong_main_main_default);
	lv_style_set_radius(&style_main_tidong_main_main_default, 0);
	lv_style_set_bg_color(&style_main_tidong_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_main_tidong_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_tidong_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_tidong_main_main_default, 255);
	lv_style_set_text_color(&style_main_tidong_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_tidong_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_tidong_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_tidong_main_main_default, 0);
	lv_style_set_text_align(&style_main_tidong_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_tidong_main_main_default, 0);
	lv_style_set_pad_right(&style_main_tidong_main_main_default, 0);
	lv_style_set_pad_top(&style_main_tidong_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_tidong_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_tidong, &style_main_tidong_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_juli
	ui->MAIN_juli = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_juli, 61, 224);
	lv_obj_set_size(ui->MAIN_juli, 100, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_juli, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_juli, "--");
	lv_label_set_long_mode(ui->MAIN_juli, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_juli_main_main_default
	static lv_style_t style_main_juli_main_main_default;
	if (style_main_juli_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_juli_main_main_default);
	else
		lv_style_init(&style_main_juli_main_main_default);
	lv_style_set_radius(&style_main_juli_main_main_default, 0);
	lv_style_set_bg_color(&style_main_juli_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_main_juli_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_juli_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_juli_main_main_default, 255);
	lv_style_set_text_color(&style_main_juli_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_juli_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_juli_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_juli_main_main_default, 0);
	lv_style_set_text_align(&style_main_juli_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_juli_main_main_default, 0);
	lv_style_set_pad_right(&style_main_juli_main_main_default, 0);
	lv_style_set_pad_top(&style_main_juli_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_juli_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_juli, &style_main_juli_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_weidu
	ui->MAIN_weidu = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_weidu, 332, 51);
	lv_obj_set_size(ui->MAIN_weidu, 140, 43);
	lv_obj_set_scrollbar_mode(ui->MAIN_weidu, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_weidu, "dd¡ãmm¡äss.ss¡å N/S");
	lv_label_set_long_mode(ui->MAIN_weidu, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_weidu_main_main_default
	static lv_style_t style_main_weidu_main_main_default;
	if (style_main_weidu_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_weidu_main_main_default);
	else
		lv_style_init(&style_main_weidu_main_main_default);
	lv_style_set_radius(&style_main_weidu_main_main_default, 0);
	lv_style_set_bg_color(&style_main_weidu_main_main_default, lv_color_make(0xde, 0x22, 0xf7));
	lv_style_set_bg_grad_color(&style_main_weidu_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_weidu_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_weidu_main_main_default, 255);
	lv_style_set_text_color(&style_main_weidu_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_weidu_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_weidu_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_weidu_main_main_default, 0);
	lv_style_set_text_align(&style_main_weidu_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_weidu_main_main_default, 0);
	lv_style_set_pad_right(&style_main_weidu_main_main_default, 0);
	lv_style_set_pad_top(&style_main_weidu_main_main_default, 4);
	lv_style_set_pad_bottom(&style_main_weidu_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_weidu, &style_main_weidu_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_jingdu
	ui->MAIN_jingdu = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_jingdu, 332, 104);
	lv_obj_set_size(ui->MAIN_jingdu, 140, 43);
	lv_obj_set_scrollbar_mode(ui->MAIN_jingdu, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_jingdu, "ddd¡ãmm¡äss.ss¡å E/W");
	lv_label_set_long_mode(ui->MAIN_jingdu, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_jingdu_main_main_default
	static lv_style_t style_main_jingdu_main_main_default;
	if (style_main_jingdu_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_jingdu_main_main_default);
	else
		lv_style_init(&style_main_jingdu_main_main_default);
	lv_style_set_radius(&style_main_jingdu_main_main_default, 0);
	lv_style_set_bg_color(&style_main_jingdu_main_main_default, lv_color_make(0xf7, 0x22, 0x7e));
	lv_style_set_bg_grad_color(&style_main_jingdu_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_jingdu_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_jingdu_main_main_default, 255);
	lv_style_set_text_color(&style_main_jingdu_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_jingdu_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_jingdu_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_jingdu_main_main_default, 0);
	lv_style_set_text_align(&style_main_jingdu_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_jingdu_main_main_default, 0);
	lv_style_set_pad_right(&style_main_jingdu_main_main_default, 0);
	lv_style_set_pad_top(&style_main_jingdu_main_main_default, 4);
	lv_style_set_pad_bottom(&style_main_jingdu_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_jingdu, &style_main_jingdu_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_label_15
	ui->MAIN_label_15 = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_label_15, 332, 162);
	lv_obj_set_size(ui->MAIN_label_15, 100, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_label_15, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_label_15, "--");
	lv_label_set_long_mode(ui->MAIN_label_15, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_label_15_main_main_default
	static lv_style_t style_main_label_15_main_main_default;
	if (style_main_label_15_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_label_15_main_main_default);
	else
		lv_style_init(&style_main_label_15_main_main_default);
	lv_style_set_radius(&style_main_label_15_main_main_default, 0);
	lv_style_set_bg_color(&style_main_label_15_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_main_label_15_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_label_15_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_label_15_main_main_default, 255);
	lv_style_set_text_color(&style_main_label_15_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_label_15_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_label_15_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_label_15_main_main_default, 0);
	lv_style_set_text_align(&style_main_label_15_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_label_15_main_main_default, 0);
	lv_style_set_pad_right(&style_main_label_15_main_main_default, 0);
	lv_style_set_pad_top(&style_main_label_15_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_label_15_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_label_15, &style_main_label_15_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_wendu
	ui->MAIN_wendu = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_wendu, 332, 162);
	lv_obj_set_size(ui->MAIN_wendu, 100, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_wendu, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_wendu, "--");
	lv_label_set_long_mode(ui->MAIN_wendu, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_wendu_main_main_default
	static lv_style_t style_main_wendu_main_main_default;
	if (style_main_wendu_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_wendu_main_main_default);
	else
		lv_style_init(&style_main_wendu_main_main_default);
	lv_style_set_radius(&style_main_wendu_main_main_default, 0);
	lv_style_set_bg_color(&style_main_wendu_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_main_wendu_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_wendu_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_wendu_main_main_default, 255);
	lv_style_set_text_color(&style_main_wendu_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_wendu_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_wendu_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_wendu_main_main_default, 0);
	lv_style_set_text_align(&style_main_wendu_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_wendu_main_main_default, 0);
	lv_style_set_pad_right(&style_main_wendu_main_main_default, 0);
	lv_style_set_pad_top(&style_main_wendu_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_wendu_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_wendu, &style_main_wendu_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_shidu
	ui->MAIN_shidu = lv_label_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_shidu, 333, 204);
	lv_obj_set_size(ui->MAIN_shidu, 100, 28);
	lv_obj_set_scrollbar_mode(ui->MAIN_shidu, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->MAIN_shidu, "--");
	lv_label_set_long_mode(ui->MAIN_shidu, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_main_shidu_main_main_default
	static lv_style_t style_main_shidu_main_main_default;
	if (style_main_shidu_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_shidu_main_main_default);
	else
		lv_style_init(&style_main_shidu_main_main_default);
	lv_style_set_radius(&style_main_shidu_main_main_default, 0);
	lv_style_set_bg_color(&style_main_shidu_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_main_shidu_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_shidu_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_shidu_main_main_default, 255);
	lv_style_set_text_color(&style_main_shidu_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_shidu_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_main_shidu_main_main_default, 2);
	lv_style_set_text_line_space(&style_main_shidu_main_main_default, 0);
	lv_style_set_text_align(&style_main_shidu_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_main_shidu_main_main_default, 0);
	lv_style_set_pad_right(&style_main_shidu_main_main_default, 0);
	lv_style_set_pad_top(&style_main_shidu_main_main_default, 6);
	lv_style_set_pad_bottom(&style_main_shidu_main_main_default, 0);
	lv_obj_add_style(ui->MAIN_shidu, &style_main_shidu_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes MAIN_fengmingqi_btn
	ui->MAIN_fengmingqi_btn = lv_btn_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_fengmingqi_btn, 7, 263);
	lv_obj_set_size(ui->MAIN_fengmingqi_btn, 100, 50);
	lv_obj_set_scrollbar_mode(ui->MAIN_fengmingqi_btn, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_main_fengmingqi_btn_main_main_default
	static lv_style_t style_main_fengmingqi_btn_main_main_default;
	if (style_main_fengmingqi_btn_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_fengmingqi_btn_main_main_default);
	else
		lv_style_init(&style_main_fengmingqi_btn_main_main_default);
	lv_style_set_radius(&style_main_fengmingqi_btn_main_main_default, 5);
	lv_style_set_bg_color(&style_main_fengmingqi_btn_main_main_default, lv_color_make(0xff, 0x00, 0xd0));
	lv_style_set_bg_grad_color(&style_main_fengmingqi_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_fengmingqi_btn_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_fengmingqi_btn_main_main_default, 255);
	lv_style_set_border_color(&style_main_fengmingqi_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_main_fengmingqi_btn_main_main_default, 0);
	lv_style_set_border_opa(&style_main_fengmingqi_btn_main_main_default, 255);
	lv_style_set_text_color(&style_main_fengmingqi_btn_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_main_fengmingqi_btn_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_align(&style_main_fengmingqi_btn_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->MAIN_fengmingqi_btn, &style_main_fengmingqi_btn_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->MAIN_fengmingqi_btn_label = lv_label_create(ui->MAIN_fengmingqi_btn);
	lv_label_set_text(ui->MAIN_fengmingqi_btn_label, "Buzzer");
	lv_obj_set_style_pad_all(ui->MAIN_fengmingqi_btn, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->MAIN_fengmingqi_btn_label, LV_ALIGN_CENTER, 0, 0);

	//Write codes MAIN_SAR_btn
	ui->MAIN_SAR_btn = lv_btn_create(ui->MAIN_cont_1);
	lv_obj_set_pos(ui->MAIN_SAR_btn, 170, 263);
	lv_obj_set_size(ui->MAIN_SAR_btn, 100, 50);
	lv_obj_set_scrollbar_mode(ui->MAIN_SAR_btn, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_main_sar_btn_main_main_default
	static lv_style_t style_main_sar_btn_main_main_default;
	if (style_main_sar_btn_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_main_sar_btn_main_main_default);
	else
		lv_style_init(&style_main_sar_btn_main_main_default);
	lv_style_set_radius(&style_main_sar_btn_main_main_default, 5);
	lv_style_set_bg_color(&style_main_sar_btn_main_main_default, lv_color_make(0xff, 0xd5, 0x00));
	lv_style_set_bg_grad_color(&style_main_sar_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_main_sar_btn_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_main_sar_btn_main_main_default, 255);
	lv_style_set_border_color(&style_main_sar_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_main_sar_btn_main_main_default, 0);
	lv_style_set_border_opa(&style_main_sar_btn_main_main_default, 255);
	lv_style_set_text_color(&style_main_sar_btn_main_main_default, lv_color_make(0xff, 0x00, 0x00));
	lv_style_set_text_font(&style_main_sar_btn_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_align(&style_main_sar_btn_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->MAIN_SAR_btn, &style_main_sar_btn_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->MAIN_SAR_btn_label = lv_label_create(ui->MAIN_SAR_btn);
	lv_label_set_text(ui->MAIN_SAR_btn_label, "SAR");
	lv_obj_set_style_pad_all(ui->MAIN_SAR_btn, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->MAIN_SAR_btn_label, LV_ALIGN_CENTER, 0, 0);

	//Init events for screen
	events_init_MAIN(ui);
}

