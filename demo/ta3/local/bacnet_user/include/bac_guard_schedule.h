/*
 * File      : guard_schedule.h
 * function  : bacnet guard_schedule ����
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-28     limian      create object
 */
#ifndef __GUARD_SCHEDULE_H__
#define __GUARD_SCHEDULE_H__

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "bac_user.h"


void Flc_Guard_Schedule_Property_Lists(const int **pRequired, const int **pOptional,
							 const int **pProprietary);
bool Flc_Guard_Schedule_Valid_Instance(uint32_t object_instance);
unsigned Flc_Guard_Schedule_Count(void);
uint32_t Flc_Guard_Schedule_Index_To_Instance(unsigned index);
unsigned Flc_Guard_Schedule_Instance_To_Index(uint32_t instance);
bool Flc_Guard_Schedule_Object_Name(uint32_t object_instance, BACNET_CHARACTER_STRING * object_name);
int Flc_Guard_Schedule_Read_Property(BACNET_READ_PROPERTY_DATA * rpdata);
bool Flc_Guard_Schedule_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data);
void Flc_Guard_Schedule_Init(void);


#endif
