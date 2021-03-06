 /*
 * File    : alarm_history.h
 * function: ˢ������
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-30     limian      first version
 *
 * note��  	
 */
 
#ifndef __ALARM_HISTORY_H__
#define __ALARM_HISTORY_H__

#include <rtthread.h>
#include "app.h"

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "bac_user.h"


void Flc_Alarm_History_Property_Lists(const int **pRequired, const int **pOptional,
							 const int **pProprietary);
bool Flc_Alarm_History_Valid_Instance(uint32_t object_instance);
unsigned Flc_Alarm_History_Count(void);
uint32_t Flc_Alarm_History_Index_To_Instance(unsigned index);
unsigned Flc_Alarm_History_Instance_To_Index(uint32_t instance);
bool Flc_Alarm_History_Object_Name(uint32_t object_instance, BACNET_CHARACTER_STRING * object_name);
int Flc_Alarm_History_Read_Property(BACNET_READ_PROPERTY_DATA * rpdata);
bool Flc_Alarm_History_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data);
void Flc_Alarm_History_Init(void);


#endif








