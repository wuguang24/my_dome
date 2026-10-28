/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "guider_fonts.h"

typedef struct
{
	lv_obj_t *MAIN;
	bool MAIN_del;
	lv_obj_t *MAIN_cont_1;
	lv_obj_t *MAIN_label15;
	lv_obj_t *MAIN_label_1;
	lv_obj_t *MAIN_label_2;
	lv_obj_t *MAIN_label_3;
	lv_obj_t *MAIN_shijian;
	lv_obj_t *MAIN_label_4;
	lv_obj_t *MAIN_label_5;
	lv_obj_t *MAIN_label_6;
	lv_obj_t *MAIN_label_7;
	lv_obj_t *MAIN_label_8;
	lv_obj_t *MAIN_label_9;
	lv_obj_t *MAIN_label_10;
	lv_obj_t *MAIN_cunzi;
	lv_obj_t *MAIN_huxi;
	lv_obj_t *MAIN_xinlv;
	lv_obj_t *MAIN_tidong;
	lv_obj_t *MAIN_juli;
	lv_obj_t *MAIN_weidu;
	lv_obj_t *MAIN_jingdu;
	lv_obj_t *MAIN_label_15;
	lv_obj_t *MAIN_wendu;
	lv_obj_t *MAIN_shidu;
	lv_obj_t *MAIN_fengmingqi_btn;
	lv_obj_t *MAIN_fengmingqi_btn_label;
	lv_obj_t *MAIN_SAR_btn;
	lv_obj_t *MAIN_SAR_btn_label;
	lv_obj_t *SAR;
	bool SAR_del;
	lv_obj_t *SAR_cont_1;
	lv_obj_t *SAR_label_1;
	lv_obj_t *SAR_label_2;
	lv_obj_t *SAR_label_3;
	lv_obj_t *SAR_label_4;
	lv_obj_t *SAR_huxi;
	lv_obj_t *SAR_xinlv;
	lv_obj_t *SAR_juli;
	lv_obj_t *SAR_label_5;
	lv_obj_t *SAR_fengmingqi_btn;
	lv_obj_t *SAR_fengmingqi_btn_label;
	lv_obj_t *SAR_led_btn;
	lv_obj_t *SAR_led_btn_label;
	lv_obj_t *SAR_label_6;
	lv_obj_t *SAR_label_7;
	lv_obj_t *SAR_wendu;
	lv_obj_t *SAR_shidu;
	lv_obj_t *SAR_MAIN_btn;
	lv_obj_t *SAR_MAIN_btn_label;
	lv_obj_t *SAR_Fault_Diagnosis_btn;
	lv_obj_t *SAR_Fault_Diagnosis_btn_label;
	lv_obj_t *Fault_Diagnosis;
	bool Fault_Diagnosis_del;
	lv_obj_t *Fault_Diagnosis_cont_1;
	lv_obj_t *Fault_Diagnosis_label_1;
	lv_obj_t *Fault_Diagnosis_label_2;
	lv_obj_t *Fault_Diagnosis_label_3;
	lv_obj_t *Fault_Diagnosis_label_4;
	lv_obj_t *Fault_Diagnosis_label_5;
	lv_obj_t *Fault_Diagnosis_fengmingqi_btn;
	lv_obj_t *Fault_Diagnosis_fengmingqi_btn_label;
	lv_obj_t *Fault_Diagnosis_led_btn;
	lv_obj_t *Fault_Diagnosis_led_btn_label;
	lv_obj_t *Fault_Diagnosis_leidazhuangtai;
	lv_obj_t *Fault_Diagnosis_GPSzhuangtai;
	lv_obj_t *Fault_Diagnosis_wenshiduzhuangtai;
	lv_obj_t *Fault_Diagnosis_wangluozhuangtai;
	lv_obj_t *Fault_Diagnosis_SAR_btn;
	lv_obj_t *Fault_Diagnosis_SAR_btn_label;
}lv_ui;

void init_scr_del_flag(lv_ui *ui);
void setup_ui(lv_ui *ui);
extern lv_ui guider_ui;
void setup_scr_MAIN(lv_ui *ui);
void setup_scr_SAR(lv_ui *ui);
void setup_scr_Fault_Diagnosis(lv_ui *ui);

#ifdef __cplusplus
}
#endif
#endif




