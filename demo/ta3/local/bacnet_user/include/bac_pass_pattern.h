/*
 * File      : pass_pattern.h
 * function  : bacnet pass_pattern ∂‘œÛ
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-25     limian      create object
 */
#ifndef __PASS_PATTERN_H__
#define __PASS_PATTERN_H__

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "bac_user.h"


void Flc_Pass_Pattern_Property_Lists(const int **pRequired, const int **pOptional,
							 const int **pProprietary);
bool Flc_Pass_Pattern_Valid_Instance(uint32_t object_instance);
unsigned Flc_Pass_Pattern_Count(void);
uint32_t Flc_Pass_Pattern_Index_To_Instance(unsigned index);
unsigned Flc_Pass_Pattern_Instance_To_Index(uint32_t instance);
bool Flc_Pass_Pattern_Object_Name(uint32_t object_instance, BACNET_CHARACTER_STRING * object_name);
int Flc_Pass_Pattern_Read_Property(BACNET_READ_PROPERTY_DATA * rpdata);
bool Flc_Pass_Pattern_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data);
void Flc_Pass_Pattern_Init(void);


#endif
