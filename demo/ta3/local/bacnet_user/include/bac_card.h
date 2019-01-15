/*
 * File      : card.h
 * function  : bacnet card ∂‘œÛ
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-28     limian      create object
 */
#ifndef __CARD_H__
#define __CARD_H__

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "bac_user.h"


void Flc_Card_Property_Lists(const int **pRequired, const int **pOptional,
							 const int **pProprietary);
bool Flc_Card_Valid_Instance(uint32_t object_instance);
unsigned Flc_Card_Count(void);
uint32_t Flc_Card_Index_To_Instance(unsigned index);
unsigned Flc_Card_Instance_To_Index(uint32_t instance);
bool Flc_Card_Object_Name(uint32_t object_instance, BACNET_CHARACTER_STRING * object_name);
int Flc_Card_Read_Property(BACNET_READ_PROPERTY_DATA * rpdata);
bool Flc_Card_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data);
void Flc_Card_Init(void);


#endif
