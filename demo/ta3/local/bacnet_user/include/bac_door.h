/*
 * File      : door.h
 * function  : bacnet door √≈∂‘œÛ
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-25     limian      create object
 */
#ifndef __DOOR_H__
#define __DOOR_H__

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "bac_user.h"

typedef struct flc_door_descr {
	uint32_t Door_Schedule_No;
	uint32_t Calendar_No;
	uint32_t Open_Valid_Time;
	uint32_t Open_Over_Time;
	uint32_t Timeout_In;
	uint32_t Timeout_Out;
	uint32_t Mode_In;
	uint32_t Mode_Out;
	TEMP_SCHEDULE Temp_Schedule[2][5];
	int Route_No_In;
	int Route_No_Out;
	bool Valid_In;
	uint32_t Privileged_Num_In;
	uint32_t Auth_Timeout_In;
	bool Valid_Out;
	uint32_t Privileged_Num_Out;
	uint32_t Auth_Timeout_Out;
	bool Restore_Ope;
	uint32_t Restore_Time_Auto;
	bool Restore_Valid;
	BACNET_TIME Restore_Time_Cyclic[5];
	uint32_t Input_Door_No[8];
}FLC_DOOR_DESCR;


void Flc_Door_Property_Lists(
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);

bool Flc_Door_Valid_Instance(uint32_t object_instance);
unsigned Flc_Door_Count(void);
uint32_t Flc_Door_Index_To_Instance(unsigned index);
unsigned Flc_Door_Instance_To_Index(uint32_t instance);
bool Flc_Door_Object_Name(uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);
int Flc_Door_Read_Property(BACNET_READ_PROPERTY_DATA * rpdata);
bool Flc_Door_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data);



void Flc_Door_Init(
    void);


#endif
