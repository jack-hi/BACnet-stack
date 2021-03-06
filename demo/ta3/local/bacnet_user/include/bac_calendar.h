/*
 * File      : calendar.h
 * function  : bacnet calendar ����
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-27     limian      create object
 */
#ifndef __CALENDAR_H__
#define __CALENDAR_H__

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "bac_user.h"


void Flc_Calendar_Property_Lists(const int **pRequired, const int **pOptional,
							 const int **pProprietary);
bool Flc_Calendar_Valid_Instance(uint32_t object_instance);
unsigned Flc_Calendar_Count(void);
uint32_t Flc_Calendar_Index_To_Instance(unsigned index);
unsigned Flc_Calendar_Instance_To_Index(uint32_t instance);
bool Flc_Calendar_Object_Name(uint32_t object_instance, BACNET_CHARACTER_STRING * object_name);
int Flc_Calendar_Read_Property(BACNET_READ_PROPERTY_DATA * rpdata);
bool Flc_Calendar_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data);
void Flc_Calendar_Init(void);


#endif
