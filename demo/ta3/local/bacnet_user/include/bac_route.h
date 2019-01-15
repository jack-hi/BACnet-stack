/*
 * File      : route.h
 * function  : bacnet route ����
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-26     limian      create object
 */
#ifndef __ROUTE_H__
#define __ROUTE_H__

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "bac_user.h"


void Flc_Route_Property_Lists(const int **pRequired, const int **pOptional,
							 const int **pProprietary);
bool Flc_Route_Valid_Instance(uint32_t object_instance);
unsigned Flc_Route_Count(void);
uint32_t Flc_Route_Index_To_Instance(unsigned index);
unsigned Flc_Route_Instance_To_Index(uint32_t instance);
bool Flc_Route_Object_Name(uint32_t object_instance, BACNET_CHARACTER_STRING * object_name);
int Flc_Route_Read_Property(BACNET_READ_PROPERTY_DATA * rpdata);
bool Flc_Route_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data);
void Flc_Route_Init(void);


#endif
