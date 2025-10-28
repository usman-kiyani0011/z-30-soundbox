/**
 * @file page_list.h
 *
 */
#pragma once

#ifndef PAGE_LIST_H
#define PAGE_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"
/**
 * ��������ѡ���б�
 * @param parent : ����ͼ
 * @param title : ����
 * @param title_height: ���� �߶�
 * @param edge_flash: ��Ե��Բ����ʽ
 * @param img_body:����ͼ
 * @return : �������б�
 */
	
lv_obj_t* page_list_create(lv_obj_t* parent, char* title,int title_height, bool edge_flash, lv_obj_t* img_body);
/**
 * ɾ��ѡ���б�
 * @param list ָ��
 */
void page_list_clean(lv_obj_t * list);

/**
 * ѡ��״̬����
 * @param �б�ָ��
 * @param track_id : ����ֵ 0...N
 * @param state : �Ƿ�ѡ��
 */
void page_list_btn_check(const lv_obj_t* list,uint32_t track_id, bool state);
/**
 * ��ȡѡ�а�ť���б������ֵ
 * @param �б�ָ��
 * @param �б�ѡ�а�ť��ָ��
 * @return �б�����ֵ -1 �������ڸ��б� �б�����:0...N
 */
int32_t page_list_get_btn_index(const lv_obj_t * list, const lv_obj_t * btn);
/**
 * ɾ���б����
 * @param �б�ָ��
 * @param Ҫɾ���б�����ֵ
 * @return ɾ���ɹ���ʧ��
 */
bool page_list_remove(const lv_obj_t* list, uint16_t index);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*PAGE_LIST_H*/
