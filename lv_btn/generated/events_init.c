/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

void events_init(lv_ui *ui)
{
}

static void MAIN_SAR_btn_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		lv_disp_t * d = lv_obj_get_disp(lv_scr_act());
		if (d->prev_scr == NULL && d->scr_to_load == NULL)
		{
			/* 先创建新页面 */
			if (guider_ui.SAR_del == true)
				setup_scr_SAR(&guider_ui);
			lv_scr_load(guider_ui.SAR);
			guider_ui.SAR_del = false;
			/* 删除旧页面释放内存 */
			if(guider_ui.MAIN) {
				lv_obj_del_async(guider_ui.MAIN);
				guider_ui.MAIN = NULL;
			}
			guider_ui.MAIN_del = true;
		}
	}
		break;
	default:
		break;
	}
}

void events_init_MAIN(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->MAIN_SAR_btn, MAIN_SAR_btn_event_handler, LV_EVENT_ALL, NULL);
}

static void SAR_MAIN_btn_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		lv_disp_t * d = lv_obj_get_disp(lv_scr_act());
		if (d->prev_scr == NULL && d->scr_to_load == NULL)
		{
			if (guider_ui.MAIN_del == true)
				setup_scr_MAIN(&guider_ui);
			lv_scr_load(guider_ui.MAIN);
			guider_ui.MAIN_del = false;
			/* 删除旧页面释放内存 */
			if(guider_ui.SAR) {
				lv_obj_del_async(guider_ui.SAR);
				guider_ui.SAR = NULL;
			}
			guider_ui.SAR_del = true;
		}
	}
		break;
	default:
		break;
	}
}

static void SAR_Fault_Diagnosis_btn_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		lv_disp_t * d = lv_obj_get_disp(lv_scr_act());
		if (d->prev_scr == NULL && d->scr_to_load == NULL)
		{
			if (guider_ui.Fault_Diagnosis_del == true)
				setup_scr_Fault_Diagnosis(&guider_ui);
			lv_scr_load(guider_ui.Fault_Diagnosis);
			guider_ui.Fault_Diagnosis_del = false;
			/* 删除旧页面释放内存 */
			if(guider_ui.SAR) {
				lv_obj_del_async(guider_ui.SAR);
				guider_ui.SAR = NULL;
			}
			guider_ui.SAR_del = true;
		}
	}
		break;
	default:
		break;
	}
}

void events_init_SAR(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->SAR_MAIN_btn, SAR_MAIN_btn_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->SAR_Fault_Diagnosis_btn, SAR_Fault_Diagnosis_btn_event_handler, LV_EVENT_ALL, NULL);
}

static void Fault_Diagnosis_SAR_btn_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		lv_disp_t * d = lv_obj_get_disp(lv_scr_act());
		if (d->prev_scr == NULL && d->scr_to_load == NULL)
		{
			if (guider_ui.SAR_del == true)
				setup_scr_SAR(&guider_ui);
			lv_scr_load(guider_ui.SAR);
			guider_ui.SAR_del = false;
			/* 删除旧页面释放内存 */
			if(guider_ui.Fault_Diagnosis) {
				lv_obj_del_async(guider_ui.Fault_Diagnosis);
				guider_ui.Fault_Diagnosis = NULL;
			}
			guider_ui.Fault_Diagnosis_del = true;
		}
	}
		break;
	default:
		break;
	}
}

void events_init_Fault_Diagnosis(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->Fault_Diagnosis_SAR_btn, Fault_Diagnosis_SAR_btn_event_handler, LV_EVENT_ALL, NULL);
}
