/*
 * File      : pass_schedule.h
 * function  : bacnet pass_schedule ����
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-28     limian      create object
 */
#ifndef __PASS_SCHEDULE_H__
#define __PASS_SCHEDULE_H__

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "bac_user.h"


void Flc_Pass_Schedule_Property_Lists(const int **pRequired, const int **pOptional,
							 const int **pProprietary);
bool Flc_Pass_Schedule_Valid_Instance(uint32_t object_instance);
unsigned Flc_Pass_Schedule_Count(void);
uint32_t Flc_Pass_Schedule_Index_To_Instance(unsigned index);
unsigned Flc_Pass_Schedule_Instance_To_Index(uint32_t instance);
bool Flc_Pass_Schedule_Object_Name(uint32_t object_instance, BACNET_CHARACTER_STRING * object_name);
int Flc_Pass_Schedule_Read_Property(BACNET_READ_PROPERTY_DATA * rpdata);
bool Flc_Pass_Schedule_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data);
void Flc_Pass_Schedule_Init(void);


#endif
