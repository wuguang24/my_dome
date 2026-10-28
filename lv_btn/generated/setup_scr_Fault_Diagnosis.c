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


void setup_scr_Fault_Diagnosis(lv_ui *ui){

	//Write codes Fault_Diagnosis
	ui->Fault_Diagnosis = lv_obj_create(NULL);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_main_main_default
	static lv_style_t style_fault_diagnosis_main_main_default;
	if (style_fault_diagnosis_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_main_main_default);
	lv_style_set_bg_color(&style_fault_diagnosis_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_opa(&style_fault_diagnosis_main_main_default, 0);
	lv_obj_add_style(ui->Fault_Diagnosis, &style_fault_diagnosis_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes Fault_Diagnosis_cont_1
	ui->Fault_Diagnosis_cont_1 = lv_obj_create(ui->Fault_Diagnosis);
	lv_obj_set_pos(ui->Fault_Diagnosis_cont_1, 0, 0);
	lv_obj_set_size(ui->Fault_Diagnosis_cont_1, 480, 320);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_cont_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_cont_1_main_main_default
	static lv_style_t style_fault_diagnosis_cont_1_main_main_default;
	if (style_fault_diagnosis_cont_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_cont_1_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_cont_1_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_cont_1_main_main_default, 0);
	lv_style_set_bg_color(&style_fault_diagnosis_cont_1_main_main_default, lv_color_make(0x7f, 0x2e, 0x2e));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_cont_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_cont_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_cont_1_main_main_default, 255);
	lv_style_set_border_color(&style_fault_diagnosis_cont_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_fault_diagnosis_cont_1_main_main_default, 2);
	lv_style_set_border_opa(&style_fault_diagnosis_cont_1_main_main_default, 255);
	lv_style_set_pad_left(&style_fault_diagnosis_cont_1_main_main_default, 0);
	lv_style_set_pad_right(&style_fault_diagnosis_cont_1_main_main_default, 0);
	lv_style_set_pad_top(&style_fault_diagnosis_cont_1_main_main_default, 0);
	lv_style_set_pad_bottom(&style_fault_diagnosis_cont_1_main_main_default, 0);
	lv_obj_add_style(ui->Fault_Diagnosis_cont_1, &style_fault_diagnosis_cont_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes Fault_Diagnosis_label_1
	ui->Fault_Diagnosis_label_1 = lv_label_create(ui->Fault_Diagnosis_cont_1);
	lv_obj_set_pos(ui->Fault_Diagnosis_label_1, 190, 3);
	lv_obj_set_size(ui->Fault_Diagnosis_label_1, 100, 32);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_label_1, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->Fault_Diagnosis_label_1, "Fault");
	lv_label_set_long_mode(ui->Fault_Diagnosis_label_1, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_label_1_main_main_default
	static lv_style_t style_fault_diagnosis_label_1_main_main_default;
	if (style_fault_diagnosis_label_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_label_1_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_label_1_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_label_1_main_main_default, 0);
	lv_style_set_bg_color(&style_fault_diagnosis_label_1_main_main_default, lv_color_make(0x22, 0x29, 0xf7));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_label_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_label_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_label_1_main_main_default, 255);
	lv_style_set_text_color(&style_fault_diagnosis_label_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_fault_diagnosis_label_1_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_fault_diagnosis_label_1_main_main_default, 2);
	lv_style_set_text_line_space(&style_fault_diagnosis_label_1_main_main_default, 0);
	lv_style_set_text_align(&style_fault_diagnosis_label_1_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_fault_diagnosis_label_1_main_main_default, 0);
	lv_style_set_pad_right(&style_fault_diagnosis_label_1_main_main_default, 0);
	lv_style_set_pad_top(&style_fault_diagnosis_label_1_main_main_default, 8);
	lv_style_set_pad_bottom(&style_fault_diagnosis_label_1_main_main_default, 0);
	lv_obj_add_style(ui->Fault_Diagnosis_label_1, &style_fault_diagnosis_label_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes Fault_Diagnosis_label_2
	ui->Fault_Diagnosis_label_2 = lv_label_create(ui->Fault_Diagnosis_cont_1);
	lv_obj_set_pos(ui->Fault_Diagnosis_label_2, 249, 97);
	lv_obj_set_size(ui->Fault_Diagnosis_label_2, 80, 32);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_label_2, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->Fault_Diagnosis_label_2, "DHT");
	lv_label_set_long_mode(ui->Fault_Diagnosis_label_2, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_label_2_main_main_default
	static lv_style_t style_fault_diagnosis_label_2_main_main_default;
	if (style_fault_diagnosis_label_2_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_label_2_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_label_2_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_label_2_main_main_default, 0);
	lv_style_set_bg_color(&style_fault_diagnosis_label_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_label_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_label_2_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_label_2_main_main_default, 255);
	lv_style_set_text_color(&style_fault_diagnosis_label_2_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_fault_diagnosis_label_2_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_fault_diagnosis_label_2_main_main_default, 2);
	lv_style_set_text_line_space(&style_fault_diagnosis_label_2_main_main_default, 0);
	lv_style_set_text_align(&style_fault_diagnosis_label_2_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_fault_diagnosis_label_2_main_main_default, 0);
	lv_style_set_pad_right(&style_fault_diagnosis_label_2_main_main_default, 0);
	lv_style_set_pad_top(&style_fault_diagnosis_label_2_main_main_default, 8);
	lv_style_set_pad_bottom(&style_fault_diagnosis_label_2_main_main_default, 0);
	lv_obj_add_style(ui->Fault_Diagnosis_label_2, &style_fault_diagnosis_label_2_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes Fault_Diagnosis_label_3
	ui->Fault_Diagnosis_label_3 = lv_label_create(ui->Fault_Diagnosis_cont_1);
	lv_obj_set_pos(ui->Fault_Diagnosis_label_3, 16, 154);
	lv_obj_set_size(ui->Fault_Diagnosis_label_3, 80, 32);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_label_3, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->Fault_Diagnosis_label_3, "GPS");
	lv_label_set_long_mode(ui->Fault_Diagnosis_label_3, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_label_3_main_main_default
	static lv_style_t style_fault_diagnosis_label_3_main_main_default;
	if (style_fault_diagnosis_label_3_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_label_3_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_label_3_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_label_3_main_main_default, 0);
	lv_style_set_bg_color(&style_fault_diagnosis_label_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_label_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_label_3_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_label_3_main_main_default, 255);
	lv_style_set_text_color(&style_fault_diagnosis_label_3_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_fault_diagnosis_label_3_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_fault_diagnosis_label_3_main_main_default, 2);
	lv_style_set_text_line_space(&style_fault_diagnosis_label_3_main_main_default, 0);
	lv_style_set_text_align(&style_fault_diagnosis_label_3_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_fault_diagnosis_label_3_main_main_default, 0);
	lv_style_set_pad_right(&style_fault_diagnosis_label_3_main_main_default, 0);
	lv_style_set_pad_top(&style_fault_diagnosis_label_3_main_main_default, 8);
	lv_style_set_pad_bottom(&style_fault_diagnosis_label_3_main_main_default, 0);
	lv_obj_add_style(ui->Fault_Diagnosis_label_3, &style_fault_diagnosis_label_3_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes Fault_Diagnosis_label_4
	ui->Fault_Diagnosis_label_4 = lv_label_create(ui->Fault_Diagnosis_cont_1);
	lv_obj_set_pos(ui->Fault_Diagnosis_label_4, 249, 154);
	lv_obj_set_size(ui->Fault_Diagnosis_label_4, 80, 32);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_label_4, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->Fault_Diagnosis_label_4, "Net");
	lv_label_set_long_mode(ui->Fault_Diagnosis_label_4, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_label_4_main_main_default
	static lv_style_t style_fault_diagnosis_label_4_main_main_default;
	if (style_fault_diagnosis_label_4_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_label_4_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_label_4_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_label_4_main_main_default, 0);
	lv_style_set_bg_color(&style_fault_diagnosis_label_4_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_label_4_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_label_4_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_label_4_main_main_default, 255);
	lv_style_set_text_color(&style_fault_diagnosis_label_4_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_fault_diagnosis_label_4_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_fault_diagnosis_label_4_main_main_default, 2);
	lv_style_set_text_line_space(&style_fault_diagnosis_label_4_main_main_default, 0);
	lv_style_set_text_align(&style_fault_diagnosis_label_4_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_fault_diagnosis_label_4_main_main_default, 0);
	lv_style_set_pad_right(&style_fault_diagnosis_label_4_main_main_default, 0);
	lv_style_set_pad_top(&style_fault_diagnosis_label_4_main_main_default, 8);
	lv_style_set_pad_bottom(&style_fault_diagnosis_label_4_main_main_default, 0);
	lv_obj_add_style(ui->Fault_Diagnosis_label_4, &style_fault_diagnosis_label_4_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes Fault_Diagnosis_label_5
	ui->Fault_Diagnosis_label_5 = lv_label_create(ui->Fault_Diagnosis_cont_1);
	lv_obj_set_pos(ui->Fault_Diagnosis_label_5, 17, 97);
	lv_obj_set_size(ui->Fault_Diagnosis_label_5, 80, 32);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_label_5, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->Fault_Diagnosis_label_5, "Radar");
	lv_label_set_long_mode(ui->Fault_Diagnosis_label_5, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_label_5_main_main_default
	static lv_style_t style_fault_diagnosis_label_5_main_main_default;
	if (style_fault_diagnosis_label_5_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_label_5_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_label_5_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_label_5_main_main_default, 0);
	lv_style_set_bg_color(&style_fault_diagnosis_label_5_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_label_5_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_label_5_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_label_5_main_main_default, 255);
	lv_style_set_text_color(&style_fault_diagnosis_label_5_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_fault_diagnosis_label_5_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_fault_diagnosis_label_5_main_main_default, 2);
	lv_style_set_text_line_space(&style_fault_diagnosis_label_5_main_main_default, 0);
	lv_style_set_text_align(&style_fault_diagnosis_label_5_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_fault_diagnosis_label_5_main_main_default, 0);
	lv_style_set_pad_right(&style_fault_diagnosis_label_5_main_main_default, 0);
	lv_style_set_pad_top(&style_fault_diagnosis_label_5_main_main_default, 8);
	lv_style_set_pad_bottom(&style_fault_diagnosis_label_5_main_main_default, 0);
	lv_obj_add_style(ui->Fault_Diagnosis_label_5, &style_fault_diagnosis_label_5_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes Fault_Diagnosis_fengmingqi_btn
	ui->Fault_Diagnosis_fengmingqi_btn = lv_btn_create(ui->Fault_Diagnosis_cont_1);
	lv_obj_set_pos(ui->Fault_Diagnosis_fengmingqi_btn, 15, 241);
	lv_obj_set_size(ui->Fault_Diagnosis_fengmingqi_btn, 100, 50);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_fengmingqi_btn, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_fengmingqi_btn_main_main_default
	static lv_style_t style_fault_diagnosis_fengmingqi_btn_main_main_default;
	if (style_fault_diagnosis_fengmingqi_btn_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_fengmingqi_btn_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_fengmingqi_btn_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_fengmingqi_btn_main_main_default, 5);
	lv_style_set_bg_color(&style_fault_diagnosis_fengmingqi_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_fengmingqi_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_fengmingqi_btn_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_fengmingqi_btn_main_main_default, 255);
	lv_style_set_border_color(&style_fault_diagnosis_fengmingqi_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_fault_diagnosis_fengmingqi_btn_main_main_default, 0);
	lv_style_set_border_opa(&style_fault_diagnosis_fengmingqi_btn_main_main_default, 255);
	lv_style_set_text_color(&style_fault_diagnosis_fengmingqi_btn_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_fault_diagnosis_fengmingqi_btn_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_align(&style_fault_diagnosis_fengmingqi_btn_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->Fault_Diagnosis_fengmingqi_btn, &style_fault_diagnosis_fengmingqi_btn_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->Fault_Diagnosis_fengmingqi_btn_label = lv_label_create(ui->Fault_Diagnosis_fengmingqi_btn);
	lv_label_set_text(ui->Fault_Diagnosis_fengmingqi_btn_label, "Buzzer");
	lv_obj_set_style_pad_all(ui->Fault_Diagnosis_fengmingqi_btn, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->Fault_Diagnosis_fengmingqi_btn_label, LV_ALIGN_CENTER, 0, 0);

	//Write codes Fault_Diagnosis_led_btn
	ui->Fault_Diagnosis_led_btn = lv_btn_create(ui->Fault_Diagnosis_cont_1);
	lv_obj_set_pos(ui->Fault_Diagnosis_led_btn, 249, 241);
	lv_obj_set_size(ui->Fault_Diagnosis_led_btn, 100, 50);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_led_btn, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_led_btn_main_main_default
	static lv_style_t style_fault_diagnosis_led_btn_main_main_default;
	if (style_fault_diagnosis_led_btn_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_led_btn_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_led_btn_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_led_btn_main_main_default, 5);
	lv_style_set_bg_color(&style_fault_diagnosis_led_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_led_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_led_btn_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_led_btn_main_main_default, 255);
	lv_style_set_border_color(&style_fault_diagnosis_led_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_fault_diagnosis_led_btn_main_main_default, 0);
	lv_style_set_border_opa(&style_fault_diagnosis_led_btn_main_main_default, 255);
	lv_style_set_text_color(&style_fault_diagnosis_led_btn_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_fault_diagnosis_led_btn_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_align(&style_fault_diagnosis_led_btn_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->Fault_Diagnosis_led_btn, &style_fault_diagnosis_led_btn_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->Fault_Diagnosis_led_btn_label = lv_label_create(ui->Fault_Diagnosis_led_btn);
	lv_label_set_text(ui->Fault_Diagnosis_led_btn_label, "LED");
	lv_obj_set_style_pad_all(ui->Fault_Diagnosis_led_btn, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->Fault_Diagnosis_led_btn_label, LV_ALIGN_CENTER, 0, 0);

	//Write codes Fault_Diagnosis_leidazhuangtai
	ui->Fault_Diagnosis_leidazhuangtai = lv_label_create(ui->Fault_Diagnosis_cont_1);
	lv_obj_set_pos(ui->Fault_Diagnosis_leidazhuangtai, 100, 97);
	lv_obj_set_size(ui->Fault_Diagnosis_leidazhuangtai, 100, 32);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_leidazhuangtai, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->Fault_Diagnosis_leidazhuangtai, "--");
	lv_label_set_long_mode(ui->Fault_Diagnosis_leidazhuangtai, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_leidazhuangtai_main_main_default
	static lv_style_t style_fault_diagnosis_leidazhuangtai_main_main_default;
	if (style_fault_diagnosis_leidazhuangtai_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_leidazhuangtai_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_leidazhuangtai_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_leidazhuangtai_main_main_default, 0);
	lv_style_set_bg_color(&style_fault_diagnosis_leidazhuangtai_main_main_default, lv_color_make(0x22, 0x29, 0xf7));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_leidazhuangtai_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_leidazhuangtai_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_leidazhuangtai_main_main_default, 255);
	lv_style_set_text_color(&style_fault_diagnosis_leidazhuangtai_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_fault_diagnosis_leidazhuangtai_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_fault_diagnosis_leidazhuangtai_main_main_default, 2);
	lv_style_set_text_line_space(&style_fault_diagnosis_leidazhuangtai_main_main_default, 0);
	lv_style_set_text_align(&style_fault_diagnosis_leidazhuangtai_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_fault_diagnosis_leidazhuangtai_main_main_default, 0);
	lv_style_set_pad_right(&style_fault_diagnosis_leidazhuangtai_main_main_default, 0);
	lv_style_set_pad_top(&style_fault_diagnosis_leidazhuangtai_main_main_default, 8);
	lv_style_set_pad_bottom(&style_fault_diagnosis_leidazhuangtai_main_main_default, 0);
	lv_obj_add_style(ui->Fault_Diagnosis_leidazhuangtai, &style_fault_diagnosis_leidazhuangtai_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes Fault_Diagnosis_GPSzhuangtai
	ui->Fault_Diagnosis_GPSzhuangtai = lv_label_create(ui->Fault_Diagnosis_cont_1);
	lv_obj_set_pos(ui->Fault_Diagnosis_GPSzhuangtai, 100, 154);
	lv_obj_set_size(ui->Fault_Diagnosis_GPSzhuangtai, 100, 32);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_GPSzhuangtai, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->Fault_Diagnosis_GPSzhuangtai, "--");
	lv_label_set_long_mode(ui->Fault_Diagnosis_GPSzhuangtai, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_gpszhuangtai_main_main_default
	static lv_style_t style_fault_diagnosis_gpszhuangtai_main_main_default;
	if (style_fault_diagnosis_gpszhuangtai_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_gpszhuangtai_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_gpszhuangtai_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_gpszhuangtai_main_main_default, 0);
	lv_style_set_bg_color(&style_fault_diagnosis_gpszhuangtai_main_main_default, lv_color_make(0x22, 0x29, 0xf7));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_gpszhuangtai_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_gpszhuangtai_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_gpszhuangtai_main_main_default, 255);
	lv_style_set_text_color(&style_fault_diagnosis_gpszhuangtai_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_fault_diagnosis_gpszhuangtai_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_fault_diagnosis_gpszhuangtai_main_main_default, 2);
	lv_style_set_text_line_space(&style_fault_diagnosis_gpszhuangtai_main_main_default, 0);
	lv_style_set_text_align(&style_fault_diagnosis_gpszhuangtai_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_fault_diagnosis_gpszhuangtai_main_main_default, 0);
	lv_style_set_pad_right(&style_fault_diagnosis_gpszhuangtai_main_main_default, 0);
	lv_style_set_pad_top(&style_fault_diagnosis_gpszhuangtai_main_main_default, 8);
	lv_style_set_pad_bottom(&style_fault_diagnosis_gpszhuangtai_main_main_default, 0);
	lv_obj_add_style(ui->Fault_Diagnosis_GPSzhuangtai, &style_fault_diagnosis_gpszhuangtai_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes Fault_Diagnosis_wenshiduzhuangtai
	ui->Fault_Diagnosis_wenshiduzhuangtai = lv_label_create(ui->Fault_Diagnosis_cont_1);
	lv_obj_set_pos(ui->Fault_Diagnosis_wenshiduzhuangtai, 332, 97);
	lv_obj_set_size(ui->Fault_Diagnosis_wenshiduzhuangtai, 100, 32);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_wenshiduzhuangtai, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->Fault_Diagnosis_wenshiduzhuangtai, "--");
	lv_label_set_long_mode(ui->Fault_Diagnosis_wenshiduzhuangtai, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_wenshiduzhuangtai_main_main_default
	static lv_style_t style_fault_diagnosis_wenshiduzhuangtai_main_main_default;
	if (style_fault_diagnosis_wenshiduzhuangtai_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, 0);
	lv_style_set_bg_color(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, lv_color_make(0x22, 0x29, 0xf7));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, 255);
	lv_style_set_text_color(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, 2);
	lv_style_set_text_line_space(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, 0);
	lv_style_set_text_align(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, 0);
	lv_style_set_pad_right(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, 0);
	lv_style_set_pad_top(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, 8);
	lv_style_set_pad_bottom(&style_fault_diagnosis_wenshiduzhuangtai_main_main_default, 0);
	lv_obj_add_style(ui->Fault_Diagnosis_wenshiduzhuangtai, &style_fault_diagnosis_wenshiduzhuangtai_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes Fault_Diagnosis_wangluozhuangtai
	ui->Fault_Diagnosis_wangluozhuangtai = lv_label_create(ui->Fault_Diagnosis_cont_1);
	lv_obj_set_pos(ui->Fault_Diagnosis_wangluozhuangtai, 332, 154);
	lv_obj_set_size(ui->Fault_Diagnosis_wangluozhuangtai, 100, 32);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_wangluozhuangtai, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->Fault_Diagnosis_wangluozhuangtai, "--");
	lv_label_set_long_mode(ui->Fault_Diagnosis_wangluozhuangtai, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_wangluozhuangtai_main_main_default
	static lv_style_t style_fault_diagnosis_wangluozhuangtai_main_main_default;
	if (style_fault_diagnosis_wangluozhuangtai_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_wangluozhuangtai_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_wangluozhuangtai_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_wangluozhuangtai_main_main_default, 0);
	lv_style_set_bg_color(&style_fault_diagnosis_wangluozhuangtai_main_main_default, lv_color_make(0x22, 0x29, 0xf7));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_wangluozhuangtai_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_wangluozhuangtai_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_wangluozhuangtai_main_main_default, 255);
	lv_style_set_text_color(&style_fault_diagnosis_wangluozhuangtai_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_fault_diagnosis_wangluozhuangtai_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_letter_space(&style_fault_diagnosis_wangluozhuangtai_main_main_default, 2);
	lv_style_set_text_line_space(&style_fault_diagnosis_wangluozhuangtai_main_main_default, 0);
	lv_style_set_text_align(&style_fault_diagnosis_wangluozhuangtai_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_fault_diagnosis_wangluozhuangtai_main_main_default, 0);
	lv_style_set_pad_right(&style_fault_diagnosis_wangluozhuangtai_main_main_default, 0);
	lv_style_set_pad_top(&style_fault_diagnosis_wangluozhuangtai_main_main_default, 8);
	lv_style_set_pad_bottom(&style_fault_diagnosis_wangluozhuangtai_main_main_default, 0);
	lv_obj_add_style(ui->Fault_Diagnosis_wangluozhuangtai, &style_fault_diagnosis_wangluozhuangtai_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes Fault_Diagnosis_SAR_btn
	ui->Fault_Diagnosis_SAR_btn = lv_btn_create(ui->Fault_Diagnosis_cont_1);
	lv_obj_set_pos(ui->Fault_Diagnosis_SAR_btn, 2, 2);
	lv_obj_set_size(ui->Fault_Diagnosis_SAR_btn, 100, 50);
	lv_obj_set_scrollbar_mode(ui->Fault_Diagnosis_SAR_btn, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_fault_diagnosis_sar_btn_main_main_default
	static lv_style_t style_fault_diagnosis_sar_btn_main_main_default;
	if (style_fault_diagnosis_sar_btn_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_fault_diagnosis_sar_btn_main_main_default);
	else
		lv_style_init(&style_fault_diagnosis_sar_btn_main_main_default);
	lv_style_set_radius(&style_fault_diagnosis_sar_btn_main_main_default, 5);
	lv_style_set_bg_color(&style_fault_diagnosis_sar_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_fault_diagnosis_sar_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_fault_diagnosis_sar_btn_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_fault_diagnosis_sar_btn_main_main_default, 255);
	lv_style_set_border_color(&style_fault_diagnosis_sar_btn_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_fault_diagnosis_sar_btn_main_main_default, 0);
	lv_style_set_border_opa(&style_fault_diagnosis_sar_btn_main_main_default, 255);
	lv_style_set_text_color(&style_fault_diagnosis_sar_btn_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_fault_diagnosis_sar_btn_main_main_default, &lv_font_simsun_16);
	lv_style_set_text_align(&style_fault_diagnosis_sar_btn_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->Fault_Diagnosis_SAR_btn, &style_fault_diagnosis_sar_btn_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->Fault_Diagnosis_SAR_btn_label = lv_label_create(ui->Fault_Diagnosis_SAR_btn);
	lv_label_set_text(ui->Fault_Diagnosis_SAR_btn_label, "Back");
	lv_obj_set_style_pad_all(ui->Fault_Diagnosis_SAR_btn, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->Fault_Diagnosis_SAR_btn_label, LV_ALIGN_CENTER, 0, 0);

	//Init events for screen
	events_init_Fault_Diagnosis(ui);
}

