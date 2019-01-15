/*
 * File      : verifier.h
 * function  : bacnet verifier ∂‘œÛ
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-25     limian      create object
 */
#ifndef __VERIFIER_H__
#define __VERIFIER_H__

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "bac_user.h"


void Flc_Verifier_Property_Lists(const int **pRequired, const int **pOptional,
							 const int **pProprietary);
bool Flc_Verifier_Valid_Instance(uint32_t object_instance);
unsigned Flc_Verifier_Count(void);
uint32_t Flc_Verifier_Index_To_Instance(unsigned index);
unsigned Flc_Verifier_Instance_To_Index(uint32_t instance);
bool Flc_Verifier_Object_Name(uint32_t object_instance, BACNET_CHARACTER_STRING * object_name);
int Flc_Verifier_Read_Property(BACNET_READ_PROPERTY_DATA * rpdata);
bool Flc_Verifier_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data);
void Flc_Verifier_Init(void);


#endif
