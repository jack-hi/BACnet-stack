/*
 * File      : global_prop.h
 * function  : bacnet global_prop FLC中一些全局属性的集合
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-26     limian      create object
 */
#ifndef __GLOBAL_PROP_H__
#define __GLOBAL_PROP_H__

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "bac_user.h"


void Flc_Global_Prop_Property_Lists(const int **pRequired, const int **pOptional,
							 const int **pProprietary);
bool Flc_Global_Prop_Valid_Instance(uint32_t object_instance);
unsigned Flc_Global_Prop_Count(void);
uint32_t Flc_Global_Prop_Index_To_Instance(unsigned index);
unsigned Flc_Global_Prop_Instance_To_Index(uint32_t instance);
bool Flc_Global_Prop_Object_Name(uint32_t object_instance, BACNET_CHARACTER_STRING * object_name);
int Flc_Global_Prop_Read_Property(BACNET_READ_PROPERTY_DATA * rpdata);
bool Flc_Global_Prop_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data);
void Flc_Global_Prop_Init(void);


#endif
